// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZombieProjectileBase.h"
#include "DamageOverTimePuddle.h"
#include "StatusEffectComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"

AZombieProjectileBase::AZombieProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication
	bReplicates = true;
	SetReplicateMovement(true);

	// Create collision sphere
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	RootComponent = CollisionSphere;
	CollisionSphere->InitSphereRadius(15.0f);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Block);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->SetNotifyRigidBodyCollision(true);

	// Create mesh
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create projectile movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionSphere;
	ProjectileMovement->InitialSpeed = 1000.0f;
	ProjectileMovement->MaxSpeed = 1500.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.5f; // Arc trajectory

	// Create trail effect
	TrailEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailEffect"));
	TrailEffect->SetupAttachment(RootComponent);
	TrailEffect->bAutoActivate = false;

	// Create audio component
	FlightAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FlightAudio"));
	FlightAudio->SetupAttachment(RootComponent);
	FlightAudio->bAutoActivate = false;

	// Set lifetime
	InitialLifeSpan = 10.0f;
}

void AZombieProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	// Bind collision
	CollisionSphere->OnComponentHit.AddDynamic(this, &AZombieProjectileBase::OnCollisionHit);

	// Initialize from config if specified
	if (!ConfigRowName.IsNone() && ProjectileConfigTable)
	{
		InitializeFromConfig(ConfigRowName);
	}

	// Start trail effect
	if (TrailEffect && TrailEffect->Template)
	{
		TrailEffect->Activate();
	}

	// Start flight sound
	if (FlightAudio && CurrentConfig.FlightSound)
	{
		FlightAudio->SetSound(CurrentConfig.FlightSound);
		FlightAudio->Play();
	}
}

void AZombieProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZombieProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombieProjectileBase, ProjectileType);
	DOREPLIFETIME(AZombieProjectileBase, DirectHitDamage);
	DOREPLIFETIME(AZombieProjectileBase, ProjectileOwner);
}

// ========================================
// PROJECTILE FUNCTIONS
// ========================================

void AZombieProjectileBase::InitializeFromConfig(FName RowName)
{
	if (!ProjectileConfigTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZombieProjectile: No config table assigned!"));
		return;
	}

	FProjectileConfigData* ConfigRow = ProjectileConfigTable->FindRow<FProjectileConfigData>(RowName, TEXT("ProjectileConfig"));
	if (!ConfigRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZombieProjectile: Config row '%s' not found!"), *RowName.ToString());
		return;
	}

	CurrentConfig = *ConfigRow;
	ConfigRowName = RowName;

	ApplyConfiguration(CurrentConfig);
}

void AZombieProjectileBase::ApplyConfiguration(const FProjectileConfigData& Config)
{
	CurrentConfig = Config;

	// Apply damage values
	DirectHitDamage = Config.DirectHitDamage;
	SplashDamage = Config.SplashDamage;
	SplashRadius = Config.SplashRadius;

	// Apply puddle settings
	bCreatePuddle = Config.bCreatePuddle;
	PuddleClass = Config.PuddleClass;
	PuddleDuration = Config.PuddleDuration;

	// Apply status effect settings
	StatusEffectTag = Config.StatusEffectTag;
	StatusEffectDuration = Config.StatusEffectDuration;
	StatusEffectStrength = Config.StatusEffectStrength;

	// Apply visuals
	if (Config.ProjectileMesh && ProjectileMesh)
	{
		ProjectileMesh->SetStaticMesh(Config.ProjectileMesh);
	}

	if (Config.ProjectileMaterial && ProjectileMesh)
	{
		ProjectileMesh->SetMaterial(0, Config.ProjectileMaterial);
	}

	if (Config.TrailEffect && TrailEffect)
	{
		TrailEffect->SetTemplate(Config.TrailEffect);
	}

	ImpactEffect = Config.ImpactEffect;
	ImpactSound = Config.ImpactSound;

	ProjectileType = Config.ProjectileType;
}

void AZombieProjectileBase::FireInDirection(const FVector& ShootDirection, float Speed)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = ShootDirection * Speed;
		ProjectileMovement->InitialSpeed = Speed;
	}
}

bool AZombieProjectileBase::FireWithArc(const FVector& TargetLocation, float ArcHeight)
{
	FVector StartLocation = GetActorLocation();
	FVector ToTarget = TargetLocation - StartLocation;
	float Distance = ToTarget.Size2D();

	if (Distance < 1.0f)
	{
		return false;
	}

	// Calculate launch velocity for arc
	float Gravity = FMath::Abs(GetWorld()->GetGravityZ()) * ProjectileMovement->ProjectileGravityScale;
	FVector HorizontalDir = FVector(ToTarget.X, ToTarget.Y, 0.0f).GetSafeNormal();

	// Calculate time to reach target
	float TimeToTarget = Distance / ProjectileMovement->InitialSpeed;

	// Calculate required vertical velocity for arc
	float VerticalVelocity = (ArcHeight / (TimeToTarget * 0.5f)) + (0.5f * Gravity * TimeToTarget * 0.5f);

	// Combine horizontal and vertical velocities
	FVector LaunchVelocity = HorizontalDir * ProjectileMovement->InitialSpeed;
	LaunchVelocity.Z = VerticalVelocity;

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = LaunchVelocity;
	}

	return true;
}

void AZombieProjectileBase::OnProjectileImpact_Implementation(const FHitResult& Hit)
{
	if (bHasImpacted)
	{
		return;
	}

	bHasImpacted = true;

	// Apply splash damage
	ApplySplashDamage(Hit.ImpactPoint);

	// Apply direct hit damage
	if (Hit.GetActor())
	{
		FPointDamageEvent DamageEvent;
		DamageEvent.Damage = DirectHitDamage;
		DamageEvent.HitInfo = Hit;

		Hit.GetActor()->TakeDamage(DirectHitDamage, DamageEvent, nullptr, ProjectileOwner);

		// Apply status effect
		ApplyStatusEffect(Hit.GetActor());
	}

	// Spawn puddle
	if (bCreatePuddle && PuddleClass)
	{
		SpawnPuddle(Hit.ImpactPoint, Hit.ImpactNormal);
	}

	// Spawn impact effect
	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	// Play impact sound
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, Hit.ImpactPoint);
	}

	// Destroy projectile
	Destroy();
}

void AZombieProjectileBase::ApplySplashDamage(const FVector& Location)
{
	if (SplashRadius <= 0.0f || SplashDamage <= 0.0f)
	{
		return;
	}

	// Find actors in splash radius
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(ProjectileOwner);

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Location,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(SplashRadius),
		QueryParams
	);

	// Apply damage to all overlapped actors
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (HitActor)
		{
			// Calculate falloff based on distance
			float Distance = FVector::Dist(Location, HitActor->GetActorLocation());
			float DamageFalloff = 1.0f - (Distance / SplashRadius);
			float ActualDamage = SplashDamage * DamageFalloff;

			FRadialDamageEvent DamageEvent;
			DamageEvent.Params.BaseDamage = ActualDamage;
			DamageEvent.Params.OuterRadius = SplashRadius;

			HitActor->TakeDamage(ActualDamage, DamageEvent, nullptr, ProjectileOwner);

			// Apply status effect (weaker for splash)
			ApplyStatusEffect(HitActor);
		}
	}
}

void AZombieProjectileBase::ApplyStatusEffect(AActor* Target)
{
	if (!Target || StatusEffectTag.IsNone())
	{
		return;
	}

	// Try to use StatusEffectComponent if available
	UStatusEffectComponent* StatusComp = Target->FindComponentByClass<UStatusEffectComponent>();
	if (StatusComp)
	{
		// Map tag name to effect type
		EStatusEffectType EffectType = EStatusEffectType::None;

		if (StatusEffectTag == TEXT("Poison"))
		{
			EffectType = EStatusEffectType::Poison;
		}
		else if (StatusEffectTag == TEXT("Acid"))
		{
			EffectType = EStatusEffectType::Acid;
		}
		else if (StatusEffectTag == TEXT("Fire"))
		{
			EffectType = EStatusEffectType::Fire;
		}
		else if (StatusEffectTag == TEXT("Slow"))
		{
			EffectType = EStatusEffectType::Slowing;
		}
		else if (StatusEffectTag == TEXT("Blind"))
		{
			EffectType = EStatusEffectType::Blinding;
		}
		else if (StatusEffectTag == TEXT("Stun"))
		{
			EffectType = EStatusEffectType::Stun;
		}
		else if (StatusEffectTag == TEXT("Weakness"))
		{
			EffectType = EStatusEffectType::Weakness;
		}

		if (EffectType != EStatusEffectType::None)
		{
			// Apply status effect with configured strength and duration
			StatusComp->ApplyStatusEffect(EffectType, StatusEffectStrength, StatusEffectDuration, ProjectileOwner);
		}
	}
	else
	{
		// Fallback: Use simple tag system if no StatusEffectComponent
		Target->Tags.AddUnique(StatusEffectTag);

		// Remove tag after duration (simple implementation)
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(
			TimerHandle,
			[Target, this]()
			{
				if (Target)
				{
					Target->Tags.Remove(StatusEffectTag);
				}
			},
			StatusEffectDuration,
			false
		);
	}
}

AActor* AZombieProjectileBase::SpawnPuddle(const FVector& Location, const FVector& Normal)
{
	if (!PuddleClass)
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = ProjectileOwner;
	SpawnParams.Instigator = Cast<APawn>(ProjectileOwner);

	FTransform SpawnTransform;
	SpawnTransform.SetLocation(Location);
	SpawnTransform.SetRotation(Normal.ToOrientationQuat());

	AActor* Puddle = GetWorld()->SpawnActor<AActor>(PuddleClass, SpawnTransform, SpawnParams);

	// Set puddle duration if it has the property
	// (In Blueprint, you'd set this via interface or cast)

	return Puddle;
}

// ========================================
// COLLISION
// ========================================

void AZombieProjectileBase::OnCollisionHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (bHasImpacted || !OtherActor || OtherActor == ProjectileOwner)
	{
		return;
	}

	OnProjectileImpact(Hit);
}
