// Copyright Epic Games, Inc. All Rights Reserved.

#include "BarricadeActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

ABarricadeActor::ABarricadeActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication
	bReplicates = true;
	SetReplicateMovement(false); // Barricades don't move

	// Create root component
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	RootComponent = CollisionBox;
	CollisionBox->SetBoxExtent(FVector(100, 50, 100));
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Block);

	// Create mesh
	BarricadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarricadeMesh"));
	BarricadeMesh->SetupAttachment(RootComponent);
	BarricadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Collision handled by box
}

void ABarricadeActor::BeginPlay()
{
	Super::BeginPlay();

	// Store original material
	if (BarricadeMesh && BarricadeMesh->GetNumMaterials() > 0)
	{
		OriginalMaterial = BarricadeMesh->GetMaterial(0);
	}

	// Initialize health
	CurrentHealth = MaxHealth;
}

void ABarricadeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABarricadeActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABarricadeActor, CurrentHealth);
	DOREPLIFETIME(ABarricadeActor, bIsDestroyed);
}

float ABarricadeActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDestroyed)
	{
		return 0.0f;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth -= ActualDamage;

	// Update visuals
	UpdateVisualDamage();

	// Play hit sound
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	// Notify
	OnBarricadeDamaged(ActualDamage, DamageCauser);
	OnDamagedByZombie(ActualDamage, DamageCauser);

	// Check for destruction
	if (CurrentHealth <= 0.0f)
	{
		DestroyBarricade();
	}

	return ActualDamage;
}

// ========================================
// TARGET INTERFACE IMPLEMENTATION
// ========================================

bool ABarricadeActor::CanBeTargeted_Implementation(AActor* AttackingZombie) const
{
	return !bIsDestroyed;
}

EZombieTargetType ABarricadeActor::GetTargetType_Implementation() const
{
	return EZombieTargetType::Barricade;
}

ETargetPriority ABarricadeActor::GetTargetPriority_Implementation() const
{
	// Higher priority when nearly destroyed (easier to finish off)
	float HealthPct = GetHealthPercentage();

	if (HealthPct < 0.25f)
	{
		return ETargetPriority::High;
	}
	else if (HealthPct < 0.5f)
	{
		return ETargetPriority::Medium;
	}
	else
	{
		return ETargetPriority::Low;
	}
}

FVector ABarricadeActor::GetTargetLocation_Implementation() const
{
	return GetActorLocation();
}

float ABarricadeActor::GetCurrentHealth_Implementation() const
{
	return CurrentHealth;
}

float ABarricadeActor::GetMaxHealth_Implementation() const
{
	return MaxHealth;
}

bool ABarricadeActor::IsTargetAlive_Implementation() const
{
	return !bIsDestroyed;
}

bool ABarricadeActor::IsVisibleToZombies_Implementation() const
{
	return !bIsDestroyed;
}

void ABarricadeActor::OnTargetedByZombie_Implementation(AActor* Zombie)
{
	if (Zombie && !TargetingZombies.Contains(Zombie))
	{
		TargetingZombies.Add(Zombie);
	}
}

void ABarricadeActor::OnUnTargetedByZombie_Implementation(AActor* Zombie)
{
	TargetingZombies.Remove(Zombie);
}

void ABarricadeActor::OnDamagedByZombie_Implementation(float Damage, AActor* Zombie)
{
	// Blueprint event
}

void ABarricadeActor::OnDestroyedByZombies_Implementation()
{
	// Blueprint event
}

float ABarricadeActor::GetDynamicPriorityModifier_Implementation(AActor* EvaluatingZombie) const
{
	// Lower health = higher priority (easier to destroy)
	float HealthPct = GetHealthPercentage();
	return FMath::Lerp(1.5f, 1.0f, HealthPct); // 1.5x at 0% health, 1.0x at 100% health
}

int32 ABarricadeActor::GetZombieTargeterCount_Implementation() const
{
	return TargetingZombies.Num();
}

bool ABarricadeActor::AllowsMultipleTargeters_Implementation() const
{
	return true; // Barricades can be attacked by multiple zombies
}

float ABarricadeActor::GetAttackRange_Implementation() const
{
	return 150.0f; // Slightly closer than normal for barricades
}

// ========================================
// BARRICADE FUNCTIONS
// ========================================

void ABarricadeActor::Repair(float Amount)
{
	if (bIsDestroyed)
	{
		return;
	}

	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
	UpdateVisualDamage();
}

void ABarricadeActor::FullRepair()
{
	if (bIsDestroyed)
	{
		return;
	}

	CurrentHealth = MaxHealth;
	UpdateVisualDamage();
}

void ABarricadeActor::DestroyBarricade()
{
	if (bIsDestroyed)
	{
		return;
	}

	bIsDestroyed = true;
	CurrentHealth = 0.0f;

	// Spawn destruction effect
	if (DestructionEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestructionEffect, GetActorLocation(), GetActorRotation());
	}

	// Play destroy sound
	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroySound, GetActorLocation());
	}

	// Notify all targeting zombies
	for (AActor* Zombie : TargetingZombies)
	{
		// Zombies will detect this is no longer a valid target on next scan
	}
	TargetingZombies.Empty();

	// Call events
	OnBarricadeDestroyed();
	OnDestroyedByZombies();

	// Disable collision
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hide mesh or spawn debris
	BarricadeMesh->SetVisibility(false);

	// Destroy after delay (for effects to play)
	SetLifeSpan(5.0f);
}

float ABarricadeActor::GetHealthPercentage() const
{
	return MaxHealth > 0.0f ? (CurrentHealth / MaxHealth) : 0.0f;
}

void ABarricadeActor::UpdateVisualDamage()
{
	if (!BarricadeMesh)
	{
		return;
	}

	float HealthPct = GetHealthPercentage();

	// Set material based on damage
	if (HealthPct <= DamageThresholdHeavy && HeavyDamageMaterial)
	{
		BarricadeMesh->SetMaterial(0, HeavyDamageMaterial);
	}
	else if (HealthPct <= DamageThresholdMedium && MediumDamageMaterial)
	{
		BarricadeMesh->SetMaterial(0, MediumDamageMaterial);
	}
	else if (HealthPct <= DamageThresholdLight && LightDamageMaterial)
	{
		BarricadeMesh->SetMaterial(0, LightDamageMaterial);
	}
	else if (OriginalMaterial)
	{
		BarricadeMesh->SetMaterial(0, OriginalMaterial);
	}
}

// ========================================
// EVENTS
// ========================================

void ABarricadeActor::OnBarricadeDamaged_Implementation(float Damage, AActor* DamageSource)
{
	// Override in Blueprint
}

void ABarricadeActor::OnBarricadeDestroyed_Implementation()
{
	// Override in Blueprint
}
