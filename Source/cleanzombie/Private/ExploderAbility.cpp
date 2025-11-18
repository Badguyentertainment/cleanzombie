// Copyright Epic Games, Inc. All Rights Reserved.

#include "ExploderAbility.h"
#include "ZombieBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "DrawDebugHelpers.h"

UExploderAbility::UExploderAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;

	AbilityName = TEXT("Exploder");
	AbilityDescription = TEXT("Suicide bomber that detonates near targets");

	// Add required tags
	AbilityTags.Add(TEXT("Exploder"));
	AbilityTags.Add(TEXT("Suicide"));
	AbilityTags.Add(TEXT("AoE"));
}

void UExploderAbility::BeginPlay()
{
	Super::BeginPlay();

	// Bind to death event if auto-detonate on death
	if (bDetonateOnDeath && OwnerZombie)
	{
		OwnerZombie->OnTakeAnyDamage.AddDynamic(this, &UExploderAbility::OnOwnerDeath);
	}
}

void UExploderAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerZombie || !bIsActive)
	{
		return;
	}

	switch (CurrentExplosionState)
	{
	case EExplosionState::Idle:
		// Check if targets are in proximity
		if (ShouldAutoDetonate())
		{
			StartCharging();
		}
		break;

	case EExplosionState::Charging:
		ChargeElapsedTime += DeltaTime;

		// Detonate when fully charged
		if (ChargeElapsedTime >= ChargeTime)
		{
			Detonate();
		}
		break;

	case EExplosionState::Detonating:
		// Explosion in progress (handled by effects)
		break;

	case EExplosionState::Exploded:
		// Already exploded, nothing to do
		break;
	}
}

// ========================================
// EXPLOSION ABILITY
// ========================================

void UExploderAbility::Detonate()
{
	if (CurrentExplosionState == EExplosionState::Exploded)
	{
		return; // Already exploded
	}

	if (CurrentExplosionState != EExplosionState::Charging && !bCanBeInterrupted)
	{
		StartCharging();
		return;
	}

	CurrentExplosionState = EExplosionState::Detonating;

	// Stop charging sound
	if (ChargingAudioComponent)
	{
		ChargingAudioComponent->Stop();
		ChargingAudioComponent = nullptr;
	}

	OnExplosionTriggered();
	ExecuteExplosion();

	CurrentExplosionState = EExplosionState::Exploded;

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("ExploderAbility: %s detonated!"), *OwnerZombie->GetName());
	}
}

void UExploderAbility::StartCharging()
{
	if (CurrentExplosionState != EExplosionState::Idle)
	{
		return;
	}

	CurrentExplosionState = EExplosionState::Charging;
	ChargeElapsedTime = 0.0f;

	// Increase movement speed while charging
	ACharacter* ZombieCharacter = Cast<ACharacter>(OwnerZombie);
	if (ZombieCharacter && ZombieCharacter->GetCharacterMovement())
	{
		float CurrentSpeed = ZombieCharacter->GetCharacterMovement()->MaxWalkSpeed;
		ZombieCharacter->GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed * ChargingSpeedMultiplier;
	}

	// Play charging sound
	if (ChargingSound && OwnerZombie)
	{
		ChargingAudioComponent = UGameplayStatics::SpawnSoundAttached(
			ChargingSound,
			OwnerZombie->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget,
			true // Stop on owner destroy
		);
	}

	OnChargingStarted();

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("ExploderAbility: %s started charging"), *OwnerZombie->GetName());
	}
}

void UExploderAbility::CancelCharge()
{
	if (CurrentExplosionState != EExplosionState::Charging)
	{
		return;
	}

	// Restore normal movement speed
	ACharacter* ZombieCharacter = Cast<ACharacter>(OwnerZombie);
	if (ZombieCharacter && ZombieCharacter->GetCharacterMovement())
	{
		float CurrentSpeed = ZombieCharacter->GetCharacterMovement()->MaxWalkSpeed;
		ZombieCharacter->GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed / ChargingSpeedMultiplier;
	}

	// Stop charging sound
	if (ChargingAudioComponent)
	{
		ChargingAudioComponent->Stop();
		ChargingAudioComponent = nullptr;
	}

	CurrentExplosionState = EExplosionState::Idle;
	ChargeElapsedTime = 0.0f;

	OnChargingCancelled();
}

float UExploderAbility::GetChargeProgress() const
{
	if (ChargeTime <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(ChargeElapsedTime / ChargeTime, 0.0f, 1.0f);
}

// ========================================
// PROTECTED FUNCTIONS
// ========================================

TArray<AActor*> UExploderAbility::FindTargetsInRange()
{
	TArray<AActor*> Targets;

	if (!OwnerZombie)
	{
		return Targets;
	}

	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(DetonationProximity);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerZombie);

	GetWorld()->SweepMultiByChannel(
		HitResults,
		OwnerZombie->GetActorLocation(),
		OwnerZombie->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		SphereShape,
		QueryParams
	);

	for (const FHitResult& Hit : HitResults)
	{
		AActor* Target = Hit.GetActor();
		if (Target)
		{
			Targets.Add(Target);
		}
	}

	return Targets;
}

bool UExploderAbility::ShouldAutoDetonate()
{
	TArray<AActor*> NearbyTargets = FindTargetsInRange();
	return NearbyTargets.Num() > 0;
}

void UExploderAbility::ExecuteExplosion()
{
	if (!OwnerZombie)
	{
		return;
	}

	FVector ExplosionLocation = OwnerZombie->GetActorLocation();

	// Find all actors in explosion radius
	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerZombie);

	GetWorld()->SweepMultiByChannel(
		HitResults,
		ExplosionLocation,
		ExplosionLocation,
		FQuat::Identity,
		ECC_Pawn,
		SphereShape,
		QueryParams
	);

	// Apply damage to all targets
	TSet<AActor*> DamagedActors;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || DamagedActors.Contains(Target))
		{
			continue;
		}

		float Distance = FVector::Dist(ExplosionLocation, Target->GetActorLocation());
		ApplyExplosionDamage(Target, Distance);
		ApplyExplosionEffects(Target);

		DamagedActors.Add(Target);
	}

	// Spawn visual effects
	SpawnExplosionEffects();

	// Apply camera shake
	if (ExplosionCameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			ExplosionCameraShake,
			ExplosionLocation,
			0.0f,
			CameraShakeRadius
		);
	}

	// Kill self if suicide explosion
	if (bSuicideExplosion && OwnerZombie)
	{
		OwnerZombie->Destroy();
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("ExploderAbility: Explosion damaged %d actors"), DamagedActors.Num());

		DrawDebugSphere(GetWorld(), ExplosionLocation, ExplosionRadius,
			16, FColor::Red, false, 3.0f, 0, 5.0f);
	}
}

void UExploderAbility::ApplyExplosionDamage(AActor* Target, float Distance)
{
	if (!Target || !OwnerZombie)
	{
		return;
	}

	// Calculate damage with falloff
	float DamageMultiplier = 1.0f;
	if (DamageFalloff > 0.0f && ExplosionRadius > 0.0f)
	{
		float DistanceRatio = Distance / ExplosionRadius;
		DamageMultiplier = 1.0f - (DistanceRatio * DamageFalloff);
		DamageMultiplier = FMath::Clamp(DamageMultiplier, 0.0f, 1.0f);
	}

	float FinalDamage = ExplosionDamage * DamageMultiplier;

	UGameplayStatics::ApplyDamage(
		Target,
		FinalDamage,
		OwnerZombie->GetInstigatorController(),
		OwnerZombie,
		UDamageType::StaticClass()
	);

	OnActorDamaged(Target, FinalDamage);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("ExploderAbility: Dealt %.0f damage to %s (Distance: %.0f)"),
			FinalDamage, *Target->GetName(), Distance);
	}
}

void UExploderAbility::ApplyExplosionEffects(AActor* Target)
{
	if (!Target || ExplosionStatusEffect == EStatusEffectType::None)
	{
		return;
	}

	UStatusEffectComponent* TargetEffects = Target->FindComponentByClass<UStatusEffectComponent>();
	if (TargetEffects)
	{
		TargetEffects->ApplyStatusEffect(
			ExplosionStatusEffect,
			StatusEffectStrength,
			StatusEffectDuration,
			OwnerZombie
		);

		// Apply type-specific effects
		switch (ExplosionType)
		{
		case EExplosionType::Fire:
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Fire, StatusEffectStrength, StatusEffectDuration, OwnerZombie);
			break;

		case EExplosionType::Acid:
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Acid, StatusEffectStrength, StatusEffectDuration, OwnerZombie);
			break;

		case EExplosionType::Poison:
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Poison, StatusEffectStrength, StatusEffectDuration, OwnerZombie);
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Diseased, StatusEffectStrength * 0.5f, StatusEffectDuration * 2.0f, OwnerZombie);
			break;

		case EExplosionType::Shrapnel:
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Bleeding, StatusEffectStrength, StatusEffectDuration, OwnerZombie);
			break;

		case EExplosionType::EMP:
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Stun, StatusEffectStrength, StatusEffectDuration * 0.3f, OwnerZombie);
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Disarmed, 1.0f, StatusEffectDuration, OwnerZombie);
			break;

		case EExplosionType::Nuclear:
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Irradiated, StatusEffectStrength * 1.5f, StatusEffectDuration * 2.0f, OwnerZombie);
			TargetEffects->ApplyStatusEffect(EStatusEffectType::Weakness, StatusEffectStrength, StatusEffectDuration, OwnerZombie);
			break;

		default:
			break;
		}
	}
}

void UExploderAbility::SpawnExplosionEffects()
{
	if (!OwnerZombie)
	{
		return;
	}

	FVector ExplosionLocation = OwnerZombie->GetActorLocation();

	// Spawn Niagara effect (preferred)
	if (ExplosionNiagara)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ExplosionNiagara,
			ExplosionLocation,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true,
			true,
			ENCPoolMethod::None,
			true
		);
	}
	// Spawn particle effect (legacy)
	else if (ExplosionParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionParticle,
			ExplosionLocation,
			FRotator::ZeroRotator,
			FVector(1.0f),
			true
		);
	}

	// Play explosion sound
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ExplosionSound,
			ExplosionLocation,
			1.0f,
			1.0f,
			0.0f,
			nullptr,
			nullptr,
			true
		);
	}
}

void UExploderAbility::OnOwnerDeath(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	// Check if owner is dead
	if (!OwnerZombie)
	{
		return;
	}

	// Trigger explosion on death
	if (bDetonateOnDeath && CurrentExplosionState != EExplosionState::Exploded)
	{
		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("ExploderAbility: %s died, triggering death explosion"), *OwnerZombie->GetName());
		}

		// Force immediate detonation
		CurrentExplosionState = EExplosionState::Charging;
		ChargeElapsedTime = ChargeTime;
		Detonate();
	}
}

// ========================================
// EVENTS
// ========================================

void UExploderAbility::OnChargingStarted_Implementation()
{
	// Override in Blueprint
}

void UExploderAbility::OnExplosionTriggered_Implementation()
{
	// Override in Blueprint
}

void UExploderAbility::OnActorDamaged_Implementation(AActor* DamagedActor, float Damage)
{
	// Override in Blueprint
}

void UExploderAbility::OnChargingCancelled_Implementation()
{
	// Override in Blueprint
}
