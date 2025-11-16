// Copyright Epic Games, Inc. All Rights Reserved.

#include "SpitterAbility.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

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

	// Calculate direction
	FVector SpitDirection = CalculateSpitDirection();

	// Spawn projectile
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(SpitDirection.ToOrientationQuat());

	AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnTransform);

	// TODO: Set projectile damage, speed, etc.

	// Reset cooldown
	TimeSinceLastSpit = 0.0f;

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("SpitterAbility: Spit at target!"));
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

	// TODO: Check line of sight

	return true;
}

FVector USpitterAbility::CalculateSpitDirection() const
{
	if (!OwnerZombie || !CurrentTarget)
	{
		return OwnerZombie ? OwnerZombie->GetActorForwardVector() : FVector::ForwardVector;
	}

	FVector ToTarget = CurrentTarget->GetActorLocation() - OwnerZombie->GetActorLocation();

	// TODO: Add prediction for moving targets if bPredictTargetMovement

	return ToTarget.GetSafeNormal();
}
