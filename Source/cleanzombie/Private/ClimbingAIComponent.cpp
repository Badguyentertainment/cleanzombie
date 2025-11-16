// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClimbingAIComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"

UClimbingAIComponent::UClimbingAIComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UClimbingAIComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter)
	{
		ClimbingMovement = Cast<UZombieClimbingMovementComponent>(OwnerCharacter->GetCharacterMovement());

		if (!ClimbingMovement)
		{
			UE_LOG(LogTemp, Warning, TEXT("ClimbingAIComponent: Owner does not have ZombieClimbingMovementComponent!"));
		}
	}
}

void UClimbingAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ClimbingMovement || !OwnerCharacter)
	{
		return;
	}

	// Track climbing time
	if (ClimbingMovement->IsClimbing())
	{
		CurrentClimbTime += DeltaTime;

		// Update climbing movement
		UpdateClimbingMovement(DeltaTime);

		// Check if should drop to attack
		if (ShouldDropToAttack())
		{
			ExecuteDropAttack();
		}
	}
	else
	{
		CurrentClimbTime = 0.0f;
	}

	// Process AI decisions
	if (bAutoClimbing)
	{
		ProcessClimbingAI(DeltaTime);
	}
}

// ========================================
// AI FUNCTIONS
// ========================================

void UClimbingAIComponent::SetTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;

	if (CurrentTarget)
	{
		LastTargetLocation = CurrentTarget->GetActorLocation();
	}
}

bool UClimbingAIComponent::ShouldClimbToTarget()
{
	if (!CurrentTarget || !ClimbingMovement || !OwnerCharacter)
	{
		return false;
	}

	// Don't climb if already climbing
	if (ClimbingMovement->IsClimbing())
	{
		return false;
	}

	// Check distance to target
	float DistanceToTarget = GetDistanceToTarget();
	if (DistanceToTarget > ClimbingConsiderationDistance)
	{
		return false;
	}

	// Check if path is blocked (if enabled)
	if (bClimbWhenPathBlocked && IsPathToTargetBlocked())
	{
		// Check if target is reachable by climbing
		float ClimbDistance;
		return IsTargetReachableByClimbing(CurrentTarget, ClimbDistance);
	}

	// Check if we prefer climbing paths
	if (bPreferClimbingPaths)
	{
		float ClimbDistance;
		return IsTargetReachableByClimbing(CurrentTarget, ClimbDistance);
	}

	return false;
}

bool UClimbingAIComponent::ShouldDropToAttack()
{
	if (!bDropToAttack || !CurrentTarget || !ClimbingMovement || !OwnerCharacter)
	{
		return false;
	}

	// Must be climbing
	if (!ClimbingMovement->IsClimbing())
	{
		return false;
	}

	// Must have been climbing for minimum time
	if (CurrentClimbTime < MinimumClimbTime)
	{
		return false;
	}

	// Check if target is below us
	if (!IsTargetBelow())
	{
		return false;
	}

	// Check horizontal distance to target
	FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = CurrentTarget->GetActorLocation();

	FVector HorizontalOffset = TargetLocation - OwnerLocation;
	HorizontalOffset.Z = 0.0f;
	float HorizontalDistance = HorizontalOffset.Size();

	if (HorizontalDistance > DropAttackDistance)
	{
		return false;
	}

	// Check vertical distance
	float VerticalDistance = OwnerLocation.Z - TargetLocation.Z;

	if (VerticalDistance < DropAttackHeightMin || VerticalDistance > MaxDropHeight)
	{
		return false;
	}

	// Check line of sight
	return HasLineOfSightToTarget();
}

bool UClimbingAIComponent::FindClimbingPath(FVector& OutClimbDirection)
{
	if (!CurrentTarget || !OwnerCharacter)
	{
		return false;
	}

	OutClimbDirection = CalculateClimbingDirection();
	return !OutClimbDirection.IsNearlyZero();
}

void UClimbingAIComponent::ExecuteDropAttack()
{
	if (!ClimbingMovement)
	{
		return;
	}

	if (bShowDebugInfo)
	{
		UE_LOG(LogTemp, Log, TEXT("ClimbingAI: Executing drop attack on target!"));
	}

	ClimbingMovement->DropFromSurface();
}

void UClimbingAIComponent::UpdateClimbingMovement(float DeltaTime)
{
	if (!ClimbingMovement || !ClimbingMovement->IsClimbing())
	{
		return;
	}

	// Calculate direction to climb
	FVector ClimbDirection = CalculateClimbingDirection();

	// Set the climb direction on the movement component
	ClimbingMovement->SetClimbDirection(ClimbDirection);

	// Debug visualization
	if (bShowDebugInfo && OwnerCharacter)
	{
		FVector Start = OwnerCharacter->GetActorLocation();
		FVector End = Start + (ClimbDirection * 100.0f);
		DrawDebugDirectionalArrow(GetWorld(), Start, End, 50.0f, FColor::Orange, false, 0.0f, 0, 3.0f);

		// Draw surface type
		FString SurfaceText = TEXT("Unknown");
		switch (ClimbingMovement->GetCurrentSurfaceType())
		{
		case EClimbingSurfaceType::Wall:
			SurfaceText = TEXT("Wall");
			break;
		case EClimbingSurfaceType::Ceiling:
			SurfaceText = TEXT("Ceiling");
			break;
		default:
			break;
		}

		DrawDebugString(GetWorld(), Start + FVector(0, 0, 100), SurfaceText, nullptr, FColor::Yellow, 0.0f);
	}
}

bool UClimbingAIComponent::IsTargetReachableByClimbing(AActor* Target, float& OutDistance)
{
	if (!Target || !ClimbingMovement || !OwnerCharacter)
	{
		return false;
	}

	FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = Target->GetActorLocation();

	// Check if there's a climbable surface between us and target
	FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
	FVector CheckLocation = OwnerLocation + (Direction * 100.0f);

	FVector SurfaceNormal;
	EClimbingSurfaceType SurfaceType;

	if (ClimbingMovement->IsLocationClimbable(CheckLocation, SurfaceNormal, SurfaceType))
	{
		OutDistance = FVector::Dist(OwnerLocation, TargetLocation);
		return true;
	}

	return false;
}

bool UClimbingAIComponent::FindNearestClimbableSurface(FVector& OutLocation, FVector& OutNormal)
{
	if (!ClimbingMovement || !OwnerCharacter)
	{
		return false;
	}

	FVector OwnerLocation = OwnerCharacter->GetActorLocation();

	// Check in multiple directions
	TArray<FVector> Directions = {
		OwnerCharacter->GetActorForwardVector(),
		-OwnerCharacter->GetActorForwardVector(),
		OwnerCharacter->GetActorRightVector(),
		-OwnerCharacter->GetActorRightVector(),
		FVector::UpVector,
	};

	float ClosestDistance = MAX_FLT;
	bool bFoundSurface = false;

	for (const FVector& Direction : Directions)
	{
		FVector CheckLocation = OwnerLocation + (Direction * 100.0f);
		FVector SurfaceNormal;
		EClimbingSurfaceType SurfaceType;

		if (ClimbingMovement->IsLocationClimbable(CheckLocation, SurfaceNormal, SurfaceType))
		{
			float Distance = FVector::Dist(OwnerLocation, CheckLocation);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				OutLocation = CheckLocation;
				OutNormal = SurfaceNormal;
				bFoundSurface = true;
			}
		}
	}

	return bFoundSurface;
}

// ========================================
// INTERNAL AI LOGIC
// ========================================

bool UClimbingAIComponent::EvaluateClimbingOpportunity()
{
	if (!ClimbingMovement || !OwnerCharacter)
	{
		return false;
	}

	// Check if we should climb to target
	if (ShouldClimbToTarget())
	{
		return true;
	}

	return false;
}

FVector UClimbingAIComponent::CalculateClimbingDirection()
{
	if (!CurrentTarget || !OwnerCharacter || !ClimbingMovement)
	{
		return FVector::ZeroVector;
	}

	FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = CurrentTarget->GetActorLocation();

	// If climbing, project target direction onto surface
	if (ClimbingMovement->IsClimbing())
	{
		FVector SurfaceNormal = ClimbingMovement->GetCurrentSurfaceNormal();
		FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();

		// Project onto surface plane
		FVector SurfaceDirection = FVector::VectorPlaneProject(ToTarget, SurfaceNormal).GetSafeNormal();

		return SurfaceDirection;
	}
	else
	{
		// Not climbing yet, just return direction to target
		return (TargetLocation - OwnerLocation).GetSafeNormal();
	}
}

bool UClimbingAIComponent::IsPathToTargetBlocked()
{
	if (!CurrentTarget || !OwnerCharacter)
	{
		return false;
	}

	// Simple raycast to check if direct path is blocked
	FVector Start = OwnerCharacter->GetActorLocation();
	FVector End = CurrentTarget->GetActorLocation();

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);
	QueryParams.AddIgnoredActor(CurrentTarget);

	bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	if (bShowDebugInfo)
	{
		DrawDebugLine(GetWorld(), Start, End, bBlocked ? FColor::Red : FColor::Green, false, 0.1f, 0, 2.0f);
	}

	return bBlocked;
}

bool UClimbingAIComponent::HasLineOfSightToTarget()
{
	if (!CurrentTarget || !OwnerCharacter)
	{
		return false;
	}

	FVector Start = OwnerCharacter->GetActorLocation();
	FVector End = CurrentTarget->GetActorLocation();

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerCharacter);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	// If we hit the target or nothing, we have line of sight
	return !bHit || Hit.GetActor() == CurrentTarget;
}

float UClimbingAIComponent::GetDistanceToTarget() const
{
	if (!CurrentTarget || !OwnerCharacter)
	{
		return MAX_FLT;
	}

	return FVector::Dist(OwnerCharacter->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool UClimbingAIComponent::IsTargetBelow() const
{
	if (!CurrentTarget || !OwnerCharacter)
	{
		return false;
	}

	return CurrentTarget->GetActorLocation().Z < OwnerCharacter->GetActorLocation().Z;
}

void UClimbingAIComponent::ProcessClimbingAI(float DeltaTime)
{
	TimeSinceLastCheck += DeltaTime;

	if (TimeSinceLastCheck < ClimbingCheckInterval)
	{
		return;
	}

	TimeSinceLastCheck = 0.0f;

	// Update target location if we have a target
	if (CurrentTarget)
	{
		LastTargetLocation = CurrentTarget->GetActorLocation();
	}

	// Evaluate if we should start climbing
	if (!ClimbingMovement->IsClimbing())
	{
		if (EvaluateClimbingOpportunity())
		{
			bWantsToClimb = true;

			if (bShowDebugInfo)
			{
				UE_LOG(LogTemp, Log, TEXT("ClimbingAI: Starting climb toward target"));
			}

			// Try to start climbing
			ClimbingMovement->TryStartClimbing();
		}
		else
		{
			bWantsToClimb = false;
		}
	}
	else
	{
		// Currently climbing - check if should stop
		if (!CurrentTarget || GetDistanceToTarget() > ClimbingConsiderationDistance * 2.0f)
		{
			if (bShowDebugInfo)
			{
				UE_LOG(LogTemp, Log, TEXT("ClimbingAI: Stopping climb - target lost or too far"));
			}

			ClimbingMovement->StopClimbing();
			bWantsToClimb = false;
		}
	}
}
