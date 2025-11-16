// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZombieClimbingMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UZombieClimbingMovementComponent::UZombieClimbingMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// Enable replication
	SetIsReplicatedByDefault(true);

	// Disable gravity when climbing (we'll handle this manually)
	GravityScale = 1.0f;
}

void UZombieClimbingMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedCharacterOwner = Cast<ACharacter>(GetOwner());
}

void UZombieClimbingMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Auto-start climbing if enabled and valid surface detected
	if (bClimbingEnabled && !IsClimbing() && MovementMode == MOVE_Walking)
	{
		// Check if we're near a climbable surface
		FHitResult Hit;
		if (DetectClimbableSurface(Hit))
		{
			// AI or player can trigger climbing here
			// For zombies, this is typically controlled by AI
		}
	}

	// Validate climbing surface periodically
	if (IsClimbing())
	{
		TimeSinceLastSurfaceCheck += DeltaTime;
		if (TimeSinceLastSurfaceCheck >= SurfaceValidationInterval)
		{
			TimeSinceLastSurfaceCheck = 0.0f;
			if (!ValidateClimbingSurface())
			{
				StopClimbing();
			}
		}
	}
}

void UZombieClimbingMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UZombieClimbingMovementComponent, ClimbingState);
}

void UZombieClimbingMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	if (!CachedCharacterOwner)
	{
		return;
	}

	switch (CustomMovementMode)
	{
	case static_cast<uint8>(ECustomMovementMode::CMOVE_WallClimbing):
		PhysWallClimbing(deltaTime, Iterations);
		break;

	case static_cast<uint8>(ECustomMovementMode::CMOVE_CeilingClimbing):
		PhysCeilingClimbing(deltaTime, Iterations);
		break;

	default:
		break;
	}
}

void UZombieClimbingMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	// Update climbing state when movement mode changes
	if (MovementMode == MOVE_Custom)
	{
		ClimbingState.bIsClimbing = true;
	}
	else if (PreviousMovementMode == MOVE_Custom)
	{
		ClimbingState.bIsClimbing = false;
		ClimbingState.CurrentSurfaceType = EClimbingSurfaceType::None;
		ClimbingState.SurfaceNormal = FVector::ZeroVector;

		// Restore normal gravity
		GravityScale = 1.0f;
	}
}

// ========================================
// CLIMBING FUNCTIONS
// ========================================

bool UZombieClimbingMovementComponent::TryStartClimbing()
{
	if (!bClimbingEnabled || IsClimbing() || !CachedCharacterOwner)
	{
		return false;
	}

	FHitResult Hit;
	if (!DetectClimbableSurface(Hit))
	{
		return false;
	}

	// Determine surface type
	EClimbingSurfaceType SurfaceType = GetSurfaceTypeFromNormal(Hit.Normal);

	if (SurfaceType == EClimbingSurfaceType::None || SurfaceType == EClimbingSurfaceType::Floor)
	{
		return false;
	}

	// Update climbing state
	ClimbingState.bIsClimbing = true;
	ClimbingState.CurrentSurfaceType = SurfaceType;
	ClimbingState.SurfaceNormal = Hit.Normal;
	ClimbingState.ClimbSpeed = (SurfaceType == EClimbingSurfaceType::Wall) ? MaxWallClimbSpeed : MaxCeilingClimbSpeed;

	// Set appropriate custom movement mode
	if (SurfaceType == EClimbingSurfaceType::Wall)
	{
		SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMovementMode::CMOVE_WallClimbing));
	}
	else if (SurfaceType == EClimbingSurfaceType::Ceiling)
	{
		SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMovementMode::CMOVE_CeilingClimbing));
		// Reverse gravity for ceiling climbing
		GravityScale = -0.5f;
	}

	return true;
}

void UZombieClimbingMovementComponent::StopClimbing()
{
	if (!IsClimbing())
	{
		return;
	}

	// Return to walking mode
	SetMovementMode(MOVE_Walking);

	// Clear climbing state
	ClimbingState.bIsClimbing = false;
	ClimbingState.CurrentSurfaceType = EClimbingSurfaceType::None;
	ClimbingState.SurfaceNormal = FVector::ZeroVector;
	ClimbingState.ClimbDirection = FVector::ZeroVector;

	// Restore gravity
	GravityScale = 1.0f;
}

bool UZombieClimbingMovementComponent::IsClimbing() const
{
	return ClimbingState.bIsClimbing && MovementMode == MOVE_Custom;
}

EClimbingSurfaceType UZombieClimbingMovementComponent::GetCurrentSurfaceType() const
{
	return ClimbingState.CurrentSurfaceType;
}

FVector UZombieClimbingMovementComponent::GetCurrentSurfaceNormal() const
{
	return ClimbingState.SurfaceNormal;
}

void UZombieClimbingMovementComponent::SetClimbDirection(FVector Direction)
{
	ClimbingState.ClimbDirection = Direction.GetSafeNormal();
}

void UZombieClimbingMovementComponent::DropFromSurface()
{
	if (!bCanDropFromSurfaces || !IsClimbing())
	{
		return;
	}

	StopClimbing();

	// Add a slight impulse away from the surface
	if (CachedCharacterOwner)
	{
		FVector DropVelocity = ClimbingState.SurfaceNormal * 200.0f;
		Velocity = DropVelocity;
		SetMovementMode(MOVE_Falling);
	}
}

bool UZombieClimbingMovementComponent::IsLocationClimbable(FVector Location, FVector& OutSurfaceNormal, EClimbingSurfaceType& OutSurfaceType)
{
	if (!CachedCharacterOwner)
	{
		return false;
	}

	FVector Start = CachedCharacterOwner->GetActorLocation();
	FVector End = Location;

	FHitResult Hit;
	if (PerformClimbingTrace(Start, End, Hit))
	{
		OutSurfaceNormal = Hit.Normal;
		OutSurfaceType = GetSurfaceTypeFromNormal(Hit.Normal);
		return OutSurfaceType != EClimbingSurfaceType::None && OutSurfaceType != EClimbingSurfaceType::Floor;
	}

	return false;
}

// ========================================
// INTERNAL CLIMBING LOGIC
// ========================================

void UZombieClimbingMovementComponent::PhysWallClimbing(float deltaTime, int32 Iterations)
{
	if (!CachedCharacterOwner || deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	// Validate we're still on a climbable surface
	if (!ValidateClimbingSurface())
	{
		StopClimbing();
		return;
	}

	// Update rotation to match surface
	UpdateClimbingRotation(deltaTime, ClimbingState.SurfaceNormal);

	// Calculate climbing velocity
	FVector ClimbVelocity = CalculateClimbingVelocity(deltaTime, ClimbingState.SurfaceNormal);

	// Apply movement
	FHitResult Hit(1.0f);
	SafeMoveUpdatedComponent(ClimbVelocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);

	// Snap to surface to maintain distance
	SnapToClimbingSurface(ClimbingState.SurfaceNormal, deltaTime);

	// Handle collision
	if (Hit.IsValidBlockingHit())
	{
		// Try to transition to new surface if enabled
		if (bAutoTransitionBetweenSurfaces)
		{
			EClimbingSurfaceType NewSurfaceType = GetSurfaceTypeFromNormal(Hit.Normal);
			if (NewSurfaceType != EClimbingSurfaceType::None && NewSurfaceType != EClimbingSurfaceType::Floor)
			{
				HandleSurfaceTransition(NewSurfaceType, Hit.Normal);
			}
			else
			{
				HandleImpact(Hit, deltaTime, ClimbVelocity);
				SlideAlongSurface(ClimbVelocity, 1.0f - Hit.Time, Hit.Normal, Hit);
			}
		}
		else
		{
			HandleImpact(Hit, deltaTime, ClimbVelocity);
			SlideAlongSurface(ClimbVelocity, 1.0f - Hit.Time, Hit.Normal, Hit);
		}
	}
}

void UZombieClimbingMovementComponent::PhysCeilingClimbing(float deltaTime, int32 Iterations)
{
	if (!CachedCharacterOwner || deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	// Validate we're still on a climbable surface
	if (!ValidateClimbingSurface())
	{
		StopClimbing();
		return;
	}

	// Update rotation to match surface (upside down)
	UpdateClimbingRotation(deltaTime, ClimbingState.SurfaceNormal);

	// Calculate climbing velocity
	FVector ClimbVelocity = CalculateClimbingVelocity(deltaTime, ClimbingState.SurfaceNormal);

	// Apply movement
	FHitResult Hit(1.0f);
	SafeMoveUpdatedComponent(ClimbVelocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);

	// Snap to surface to maintain distance
	SnapToClimbingSurface(ClimbingState.SurfaceNormal, deltaTime);

	// Handle collision
	if (Hit.IsValidBlockingHit())
	{
		if (bAutoTransitionBetweenSurfaces)
		{
			EClimbingSurfaceType NewSurfaceType = GetSurfaceTypeFromNormal(Hit.Normal);
			if (NewSurfaceType != EClimbingSurfaceType::None && NewSurfaceType != EClimbingSurfaceType::Floor)
			{
				HandleSurfaceTransition(NewSurfaceType, Hit.Normal);
			}
			else
			{
				HandleImpact(Hit, deltaTime, ClimbVelocity);
				SlideAlongSurface(ClimbVelocity, 1.0f - Hit.Time, Hit.Normal, Hit);
			}
		}
		else
		{
			HandleImpact(Hit, deltaTime, ClimbVelocity);
			SlideAlongSurface(ClimbVelocity, 1.0f - Hit.Time, Hit.Normal, Hit);
		}
	}
}

bool UZombieClimbingMovementComponent::DetectClimbableSurface(FHitResult& OutHit)
{
	if (!CachedCharacterOwner)
	{
		return false;
	}

	FVector Start = CachedCharacterOwner->GetActorLocation();
	FVector Forward = CachedCharacterOwner->GetActorForwardVector();
	FVector End = Start + (Forward * ClimbableDetectionDistance);

	return PerformClimbingTrace(Start, End, OutHit);
}

EClimbingSurfaceType UZombieClimbingMovementComponent::GetSurfaceTypeFromNormal(const FVector& Normal) const
{
	// Calculate angle from horizontal (up vector)
	float AngleFromHorizontal = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));

	if (AngleFromHorizontal < (90.0f - WallMinAngle))
	{
		// Mostly pointing up - floor
		return EClimbingSurfaceType::Floor;
	}
	else if (AngleFromHorizontal >= WallMinAngle && AngleFromHorizontal < 90.0f)
	{
		// Angled surface - treat as wall
		return EClimbingSurfaceType::Wall;
	}
	else if (AngleFromHorizontal >= 90.0f && AngleFromHorizontal <= CeilingMaxAngle)
	{
		// Mostly pointing sideways - wall
		return EClimbingSurfaceType::Wall;
	}
	else if (AngleFromHorizontal > CeilingMaxAngle)
	{
		// Mostly pointing down - ceiling
		return EClimbingSurfaceType::Ceiling;
	}

	return EClimbingSurfaceType::None;
}

void UZombieClimbingMovementComponent::UpdateClimbingRotation(float DeltaTime, const FVector& SurfaceNormal)
{
	if (!CachedCharacterOwner)
	{
		return;
	}

	// Calculate target rotation to align with surface
	FVector UpVector = SurfaceNormal;
	FVector ForwardVector = CachedCharacterOwner->GetActorForwardVector();

	// Project forward vector onto the surface plane
	ForwardVector = FVector::VectorPlaneProject(ForwardVector, SurfaceNormal).GetSafeNormal();

	// If forward vector is near zero, use right vector instead
	if (ForwardVector.IsNearlyZero())
	{
		ForwardVector = FVector::VectorPlaneProject(CachedCharacterOwner->GetActorRightVector(), SurfaceNormal).GetSafeNormal();
	}

	FRotator TargetRotation = UKismetMathLibrary::MakeRotFromXZ(ForwardVector, UpVector);
	FRotator CurrentRotation = CachedCharacterOwner->GetActorRotation();

	// Smoothly interpolate to target rotation
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, RotationSpeed);
	CachedCharacterOwner->SetActorRotation(NewRotation);
}

void UZombieClimbingMovementComponent::SnapToClimbingSurface(const FVector& SurfaceNormal, float DeltaTime)
{
	if (!CachedCharacterOwner)
	{
		return;
	}

	// Trace from character to find surface
	FVector Start = CachedCharacterOwner->GetActorLocation();
	FVector End = Start - (SurfaceNormal * (ClimbableDetectionDistance * 2.0f));

	FHitResult Hit;
	if (PerformClimbingTrace(Start, End, Hit))
	{
		// Calculate desired position (offset from surface)
		FVector DesiredPosition = Hit.ImpactPoint + (Hit.Normal * SurfaceOffset);
		FVector CurrentPosition = CachedCharacterOwner->GetActorLocation();

		// Smoothly move toward desired position
		FVector NewPosition = FMath::VInterpTo(CurrentPosition, DesiredPosition, DeltaTime, RotationSpeed);

		FHitResult MoveHit(1.0f);
		SafeMoveUpdatedComponent(NewPosition - CurrentPosition, UpdatedComponent->GetComponentQuat(), false, MoveHit);
	}
}

FVector UZombieClimbingMovementComponent::CalculateClimbingVelocity(float DeltaTime, const FVector& SurfaceNormal)
{
	if (!CachedCharacterOwner)
	{
		return FVector::ZeroVector;
	}

	// Get desired movement direction
	FVector MovementDirection = GetClimbingMovementDirection();

	if (MovementDirection.IsNearlyZero())
	{
		// Apply deceleration
		Velocity = FMath::VInterpTo(Velocity, FVector::ZeroVector, DeltaTime, ClimbingDeceleration / 100.0f);
		return Velocity;
	}

	// Project movement onto surface plane
	FVector SurfaceMovement = FVector::VectorPlaneProject(MovementDirection, SurfaceNormal).GetSafeNormal();

	// Calculate target velocity
	float MaxSpeed = (ClimbingState.CurrentSurfaceType == EClimbingSurfaceType::Wall) ? MaxWallClimbSpeed : MaxCeilingClimbSpeed;
	FVector TargetVelocity = SurfaceMovement * MaxSpeed;

	// Apply acceleration
	Velocity = FMath::VInterpTo(Velocity, TargetVelocity, DeltaTime, ClimbingAcceleration / 100.0f);

	return Velocity;
}

bool UZombieClimbingMovementComponent::ValidateClimbingSurface()
{
	if (!CachedCharacterOwner)
	{
		return false;
	}

	// Trace from character toward surface
	FVector Start = CachedCharacterOwner->GetActorLocation();
	FVector End = Start - (ClimbingState.SurfaceNormal * (ClimbableDetectionDistance * 1.5f));

	FHitResult Hit;
	if (PerformClimbingTrace(Start, End, Hit))
	{
		// Update surface normal
		ClimbingState.SurfaceNormal = Hit.Normal;

		// Check if surface type has changed
		EClimbingSurfaceType NewSurfaceType = GetSurfaceTypeFromNormal(Hit.Normal);

		if (bAutoTransitionBetweenSurfaces && NewSurfaceType != ClimbingState.CurrentSurfaceType)
		{
			if (NewSurfaceType != EClimbingSurfaceType::None && NewSurfaceType != EClimbingSurfaceType::Floor)
			{
				HandleSurfaceTransition(NewSurfaceType, Hit.Normal);
				return true;
			}
		}

		return NewSurfaceType == ClimbingState.CurrentSurfaceType;
	}

	return false;
}

void UZombieClimbingMovementComponent::HandleSurfaceTransition(EClimbingSurfaceType NewSurfaceType, const FVector& NewNormal)
{
	if (NewSurfaceType == ClimbingState.CurrentSurfaceType)
	{
		return;
	}

	// Update climbing state
	EClimbingSurfaceType OldSurfaceType = ClimbingState.CurrentSurfaceType;
	ClimbingState.CurrentSurfaceType = NewSurfaceType;
	ClimbingState.SurfaceNormal = NewNormal;

	// Update movement mode
	if (NewSurfaceType == EClimbingSurfaceType::Wall)
	{
		SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMovementMode::CMOVE_WallClimbing));
		GravityScale = 0.0f;
		ClimbingState.ClimbSpeed = MaxWallClimbSpeed;
	}
	else if (NewSurfaceType == EClimbingSurfaceType::Ceiling)
	{
		SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMovementMode::CMOVE_CeilingClimbing));
		GravityScale = -0.5f;
		ClimbingState.ClimbSpeed = MaxCeilingClimbSpeed;
	}
}

bool UZombieClimbingMovementComponent::PerformClimbingTrace(const FVector& Start, const FVector& End, FHitResult& OutHit)
{
	if (!CachedCharacterOwner)
	{
		return false;
	}

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedCharacterOwner);
	QueryParams.bTraceComplex = false;

	bool bHit = false;

	if (ClimbableDetectionRadius > 0.0f)
	{
		// Sphere trace
		bHit = GetWorld()->SweepSingleByChannel(
			OutHit,
			Start,
			End,
			FQuat::Identity,
			ClimbableTraceChannel,
			FCollisionShape::MakeSphere(ClimbableDetectionRadius),
			QueryParams
		);
	}
	else
	{
		// Line trace
		bHit = GetWorld()->LineTraceSingleByChannel(
			OutHit,
			Start,
			End,
			ClimbableTraceChannel,
			QueryParams
		);
	}

	// Debug visualization
	if (bShowDebugTraces)
	{
		if (ClimbableDetectionRadius > 0.0f)
		{
			DrawDebugSphere(GetWorld(), Start, ClimbableDetectionRadius, 12, FColor::Yellow, false, 0.1f);
			DrawDebugSphere(GetWorld(), End, ClimbableDetectionRadius, 12, bHit ? FColor::Green : FColor::Red, false, 0.1f);
			DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.1f, 0, 2.0f);
		}
		else
		{
			DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.1f, 0, 2.0f);
		}

		if (bHit)
		{
			DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 10.0f, FColor::Cyan, false, 0.1f);
			DrawDebugLine(GetWorld(), OutHit.ImpactPoint, OutHit.ImpactPoint + (OutHit.Normal * 50.0f), FColor::Blue, false, 0.1f, 0, 3.0f);
		}
	}

	return bHit;
}

FVector UZombieClimbingMovementComponent::GetClimbingMovementDirection() const
{
	// If AI has set a specific climb direction, use that
	if (!ClimbingState.ClimbDirection.IsNearlyZero())
	{
		return ClimbingState.ClimbDirection;
	}

	// Otherwise use acceleration input (from AI or player controller)
	if (!GetCurrentAcceleration().IsNearlyZero())
	{
		return GetCurrentAcceleration().GetSafeNormal();
	}

	return FVector::ZeroVector;
}
