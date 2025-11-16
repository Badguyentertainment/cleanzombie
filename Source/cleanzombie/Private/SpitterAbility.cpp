// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpitterAbility.h"
#include "ZombieProjectileBase.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

USpitterAbility::USpitterAbility()
{
	AbilityName = FText::FromString(TEXT("Acid Spit"));
	AbilityDescription = FText::FromString(TEXT("Ranged acid projectile attack"));
	AbilityPriority = 50; // Medium priority
	bCanRunConcurrently = false; // Can't spit while doing other actions

	// Add blocking tags (can't spit while climbing, etc.)
	BlockingTags.Add(TEXT("Climbing"));
	BlockingTags.Add(TEXT("Stunned"));

	// Add ability tags
	AbilityTags.Add(TEXT("Attacking"));
	AbilityTags.Add(TEXT("Ranged"));
}

void USpitterAbility::InitializeAbility_Implementation()
{
	Super::InitializeAbility_Implementation();

	TimeSinceLastSpit = SpitCooldown; // Ready to spit immediately
}

bool USpitterAbility::ActivateAbility_Implementation()
{
	if (!Super::ActivateAbility_Implementation())
	{
		return false;
	}

	// Spit at target
	SpitAtTarget();

	// Deactivate immediately (it's an instant action)
	DeactivateAbility();

	return true;
}

void USpitterAbility::DeactivateAbility_Implementation()
{
	Super::DeactivateAbility_Implementation();
}

void USpitterAbility::UpdateAbility_Implementation(float DeltaTime)
{
	Super::UpdateAbility_Implementation(DeltaTime);

	// Update cooldown
	TimeSinceLastSpit += DeltaTime;

	// Auto-spit if target in range and cooldown ready
	if (CanSpitAtTarget())
	{
		ActivateAbility();
	}
}

bool USpitterAbility::CanActivate_Implementation() const
{
	if (!Super::CanActivate_Implementation())
	{
		return false;
	}

	return CanSpitAtTarget();
}

// ========================================
// SPITTER FUNCTIONS
// ========================================

void USpitterAbility::SpitAtTarget()
{
	if (!OwnerZombie || !ProjectileClass || !HasValidTarget())
	{
		return;
	}

	// Get spawn location (from mouth socket if available)
	FVector SpawnLocation = OwnerZombie->GetActorLocation() + FVector(0, 0, 50);

	USkeletalMeshComponent* Mesh = OwnerZombie->GetMesh();
	if (Mesh && Mesh->DoesSocketExist(SpitSocketName))
	{
		SpawnLocation = Mesh->GetSocketLocation(SpitSocketName);
	}

	// Calculate target location with prediction
	FVector TargetLocation = CurrentTarget->GetActorLocation();

	if (bPredictTargetMovement)
	{
		// Predict where target will be
		ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget);
		if (TargetCharacter && TargetCharacter->GetCharacterMovement())
		{
			FVector TargetVelocity = TargetCharacter->GetCharacterMovement()->Velocity;
			float Distance = FVector::Dist(SpawnLocation, TargetLocation);
			float TimeToHit = Distance / ProjectileSpeed;

			// Add predicted movement
			TargetLocation += TargetVelocity * TimeToHit;
		}
	}

	// Spawn projectile
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(FRotator::ZeroRotator);

	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTransform);

	// Configure projectile if it's a ZombieProjectileBase
	if (AZombieProjectileBase* ZombieProjectile = Cast<AZombieProjectileBase>(Projectile))
	{
		// Set owner for damage attribution
		ZombieProjectile->SetOwner(OwnerZombie);
		ZombieProjectile->ProjectileOwner = OwnerZombie;

		// Fire with arc trajectory
		float Distance = FVector::Dist(SpawnLocation, TargetLocation);
		float ArcHeight = FMath::Lerp(100.0f, 300.0f, Distance / MaxSpitRange);

		ZombieProjectile->FireWithArc(TargetLocation, ArcHeight);

		if (bShowDebug)
		{
			DrawDebugLine(GetWorld(), SpawnLocation, TargetLocation, FColor::Green, false, 2.0f, 0, 2.0f);
			DrawDebugSphere(GetWorld(), TargetLocation, 50.0f, 12, FColor::Red, false, 2.0f);
		}
	}

	// Reset cooldown
	TimeSinceLastSpit = 0.0f;

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("SpitterAbility: Spit at target %s at distance %.1f"),
			*CurrentTarget->GetName(), FVector::Dist(SpawnLocation, TargetLocation));
	}
}

bool USpitterAbility::CanSpitAtTarget() const
{
	if (!HasValidTarget() || TimeSinceLastSpit < SpitCooldown)
	{
		return false;
	}

	if (!OwnerZombie)
	{
		return false;
	}

	// Check distance
	float Distance = FVector::Dist(OwnerZombie->GetActorLocation(), CurrentTarget->GetActorLocation());

	if (Distance < MinSpitRange || Distance > MaxSpitRange)
	{
		return false;
	}

	// Check line of sight
	FVector StartLocation = OwnerZombie->GetActorLocation() + FVector(0, 0, 50);
	FVector EndLocation = CurrentTarget->GetActorLocation();

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerZombie);
	QueryParams.AddIgnoredActor(CurrentTarget);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// If we hit something between us and the target, we don't have LOS
	if (bHit)
	{
		return false;
	}

	return true;
}

FVector USpitterAbility::CalculateSpitDirection() const
{
	if (!OwnerZombie || !CurrentTarget)
	{
		return OwnerZombie ? OwnerZombie->GetActorForwardVector() : FVector::ForwardVector;
	}

	FVector TargetLocation = CurrentTarget->GetActorLocation();

	// Apply target prediction if enabled
	if (bPredictTargetMovement)
	{
		ACharacter* TargetCharacter = Cast<ACharacter>(CurrentTarget);
		if (TargetCharacter && TargetCharacter->GetCharacterMovement())
		{
			FVector TargetVelocity = TargetCharacter->GetCharacterMovement()->Velocity;
			float Distance = FVector::Dist(OwnerZombie->GetActorLocation(), TargetLocation);
			float TimeToHit = Distance / FMath::Max(ProjectileSpeed, 100.0f);

			// Add predicted movement
			TargetLocation += TargetVelocity * TimeToHit;
		}
	}

	FVector ToTarget = TargetLocation - OwnerZombie->GetActorLocation();
	return ToTarget.GetSafeNormal();
}
