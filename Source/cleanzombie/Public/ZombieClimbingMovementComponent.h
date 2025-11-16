// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ZombieClimbingMovementComponent.generated.h"

/**
 * Climbing surface type enum
 */
UENUM(BlueprintType)
enum class EClimbingSurfaceType : uint8
{
	None UMETA(DisplayName = "None"),
	Wall UMETA(DisplayName = "Wall"),
	Ceiling UMETA(DisplayName = "Ceiling"),
	Floor UMETA(DisplayName = "Floor")
};

/**
 * Custom movement modes for climbing
 */
UENUM(BlueprintType)
enum class ECustomMovementMode : uint8
{
	CMOVE_Climbing UMETA(DisplayName = "Climbing"),
	CMOVE_WallClimbing UMETA(DisplayName = "Wall Climbing"),
	CMOVE_CeilingClimbing UMETA(DisplayName = "Ceiling Climbing")
};

/**
 * Climbing state information structure
 */
USTRUCT(BlueprintType)
struct FClimbingState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Climbing")
	bool bIsClimbing;

	UPROPERTY(BlueprintReadWrite, Category = "Climbing")
	EClimbingSurfaceType CurrentSurfaceType;

	UPROPERTY(BlueprintReadWrite, Category = "Climbing")
	FVector SurfaceNormal;

	UPROPERTY(BlueprintReadWrite, Category = "Climbing")
	FVector ClimbDirection;

	UPROPERTY(BlueprintReadWrite, Category = "Climbing")
	float ClimbSpeed;

	FClimbingState()
		: bIsClimbing(false)
		, CurrentSurfaceType(EClimbingSurfaceType::None)
		, SurfaceNormal(FVector::ZeroVector)
		, ClimbDirection(FVector::ZeroVector)
		, ClimbSpeed(0.f)
	{
	}
};

/**
 * Enhanced Character Movement Component with wall and ceiling climbing capabilities
 * Designed for zombie AI characters in multiplayer environments
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UZombieClimbingMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UZombieClimbingMovementComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Override movement physics
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

	// Network replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	// ========================================
	// CLIMBING CONFIGURATION PARAMETERS
	// ========================================

	/** Maximum speed when climbing on walls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Speed", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MaxWallClimbSpeed = 150.0f;

	/** Maximum speed when climbing on ceilings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Speed", meta = (ClampMin = "0.0", ClampMax = "1000.0"))
	float MaxCeilingClimbSpeed = 120.0f;

	/** Acceleration when climbing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Speed", meta = (ClampMin = "0.0"))
	float ClimbingAcceleration = 500.0f;

	/** Deceleration when climbing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Speed", meta = (ClampMin = "0.0"))
	float ClimbingDeceleration = 1000.0f;

	/** Maximum distance to detect climbable surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Detection", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float ClimbableDetectionDistance = 100.0f;

	/** Radius for sphere traces when detecting surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Detection", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ClimbableDetectionRadius = 30.0f;

	/** Minimum angle (in degrees) for a surface to be considered a wall (from horizontal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Detection", meta = (ClampMin = "0.0", ClampMax = "90.0"))
	float WallMinAngle = 60.0f;

	/** Maximum angle (in degrees) for a surface to be considered a ceiling (from horizontal) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Detection", meta = (ClampMin = "90.0", ClampMax = "180.0"))
	float CeilingMaxAngle = 135.0f;

	/** How quickly to rotate to match surface normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Rotation", meta = (ClampMin = "0.0", ClampMax = "30.0"))
	float RotationSpeed = 10.0f;

	/** Distance to maintain from climbing surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Movement", meta = (ClampMin = "0.0", ClampMax = "200.0"))
	float SurfaceOffset = 50.0f;

	/** Enable debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Debug")
	bool bShowDebugTraces = false;

	/** Trace channel for detecting climbable surfaces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Detection")
	TEnumAsByte<ECollisionChannel> ClimbableTraceChannel = ECC_Visibility;

	/** Can this character drop from climbing surfaces? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Abilities")
	bool bCanDropFromSurfaces = true;

	/** Can automatically transition between different surface types? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Abilities")
	bool bAutoTransitionBetweenSurfaces = true;

	// ========================================
	// CLIMBING STATE
	// ========================================

	/** Current climbing state (replicated) */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Zombie Climbing|State")
	FClimbingState ClimbingState;

	/** Is climbing enabled for this component? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Climbing|Abilities")
	bool bClimbingEnabled = true;

	// ========================================
	// CLIMBING FUNCTIONS
	// ========================================

	/** Attempt to start climbing */
	UFUNCTION(BlueprintCallable, Category = "Zombie Climbing")
	bool TryStartClimbing();

	/** Stop climbing and return to walking */
	UFUNCTION(BlueprintCallable, Category = "Zombie Climbing")
	void StopClimbing();

	/** Check if currently climbing */
	UFUNCTION(BlueprintPure, Category = "Zombie Climbing")
	bool IsClimbing() const;

	/** Get current surface type being climbed */
	UFUNCTION(BlueprintPure, Category = "Zombie Climbing")
	EClimbingSurfaceType GetCurrentSurfaceType() const;

	/** Get current surface normal */
	UFUNCTION(BlueprintPure, Category = "Zombie Climbing")
	FVector GetCurrentSurfaceNormal() const;

	/** Manually set climb direction (for AI pathfinding) */
	UFUNCTION(BlueprintCallable, Category = "Zombie Climbing")
	void SetClimbDirection(FVector Direction);

	/** Drop from current climbing surface */
	UFUNCTION(BlueprintCallable, Category = "Zombie Climbing")
	void DropFromSurface();

	/** Check if a surface at a location is climbable */
	UFUNCTION(BlueprintCallable, Category = "Zombie Climbing")
	bool IsLocationClimbable(FVector Location, FVector& OutSurfaceNormal, EClimbingSurfaceType& OutSurfaceType);

protected:
	// ========================================
	// INTERNAL CLIMBING LOGIC
	// ========================================

	/** Physics update for wall climbing */
	void PhysWallClimbing(float deltaTime, int32 Iterations);

	/** Physics update for ceiling climbing */
	void PhysCeilingClimbing(float deltaTime, int32 Iterations);

	/** Detect climbable surface in front of character */
	bool DetectClimbableSurface(FHitResult& OutHit);

	/** Determine surface type from normal vector */
	EClimbingSurfaceType GetSurfaceTypeFromNormal(const FVector& Normal) const;

	/** Update rotation to match surface */
	void UpdateClimbingRotation(float DeltaTime, const FVector& SurfaceNormal);

	/** Snap character to climbing surface */
	void SnapToClimbingSurface(const FVector& SurfaceNormal, float DeltaTime);

	/** Calculate movement along climbing surface */
	FVector CalculateClimbingVelocity(float DeltaTime, const FVector& SurfaceNormal);

	/** Check if still on valid climbing surface */
	bool ValidateClimbingSurface();

	/** Handle transition between surface types */
	void HandleSurfaceTransition(EClimbingSurfaceType NewSurfaceType, const FVector& NewNormal);

	/** Perform trace checks for surface detection */
	bool PerformClimbingTrace(const FVector& Start, const FVector& End, FHitResult& OutHit);

	/** Calculate desired movement direction on surface */
	FVector GetClimbingMovementDirection() const;

private:
	/** Time since last surface validation check */
	float TimeSinceLastSurfaceCheck = 0.0f;

	/** How often to validate climbing surface (in seconds) */
	const float SurfaceValidationInterval = 0.1f;

	/** Cached character owner reference */
	UPROPERTY()
	ACharacter* CachedCharacterOwner;
};
