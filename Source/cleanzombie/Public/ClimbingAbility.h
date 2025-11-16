// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "ZombieClimbingMovementComponent.h"
#include "ClimbingAbility.generated.h"

/**
 * Climbing ability - enables wall and ceiling climbing for zombies
 * Refactored version that integrates with the modular ability framework
 * Use this instead of ClimbingAIComponent for new implementations
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UClimbingAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	UClimbingAbility();

protected:
	virtual void BeginPlay() override;

public:
	// ========================================
	// ABILITY OVERRIDES
	// ========================================

	virtual void InitializeAbility_Implementation() override;
	virtual bool ActivateAbility_Implementation() override;
	virtual void DeactivateAbility_Implementation() override;
	virtual void UpdateAbility_Implementation(float DeltaTime) override;
	virtual bool CanActivate_Implementation() const override;

	virtual void OnZombieDetectedTarget_Implementation(AActor* DetectedActor) override;
	virtual void OnZombieLostTarget_Implementation(AActor* LostActor) override;

	// ========================================
	// CLIMBING CONFIGURATION
	// ========================================

	/** Should AI automatically decide when to climb? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Behavior")
	bool bAutoClimbing = true;

	/** How often to check for climbing opportunities (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Behavior", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float ClimbingCheckInterval = 0.5f;

	/** Prefer climbing to reach target if path is blocked? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Behavior")
	bool bClimbWhenPathBlocked = true;

	/** Distance at which to consider climbing to reach target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Behavior", meta = (ClampMin = "0.0"))
	float ClimbingConsiderationDistance = 500.0f;

	/** Should drop from ceiling/walls to attack targets below? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Attack")
	bool bDropToAttack = true;

	/** Distance from target to drop for attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Attack", meta = (ClampMin = "0.0"))
	float DropAttackDistance = 300.0f;

	/** Vertical distance threshold for drop attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Attack", meta = (ClampMin = "0.0"))
	float DropAttackHeightMin = 100.0f;

	/** Maximum height from which to drop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Attack", meta = (ClampMin = "0.0"))
	float MaxDropHeight = 1000.0f;

	/** Prefer walls over ground when both paths available? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Behavior")
	bool bPreferClimbingPaths = false;

	/** Minimum time to spend climbing before considering dropping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing Ability|Behavior", meta = (ClampMin = "0.0"))
	float MinimumClimbTime = 2.0f;

	// ========================================
	// CLIMBING STATE
	// ========================================

	/** Is AI currently trying to climb? */
	UPROPERTY(BlueprintReadOnly, Category = "Climbing Ability|State")
	bool bWantsToClimb = false;

	/** Time spent in current climbing session */
	UPROPERTY(BlueprintReadOnly, Category = "Climbing Ability|State")
	float CurrentClimbTime = 0.0f;

	// ========================================
	// CLIMBING FUNCTIONS
	// ========================================

	/** Check if should start climbing toward target */
	UFUNCTION(BlueprintCallable, Category = "Climbing Ability")
	bool ShouldClimbToTarget();

	/** Check if should drop to attack target */
	UFUNCTION(BlueprintCallable, Category = "Climbing Ability")
	bool ShouldDropToAttack();

	/** Find climbing path to target */
	UFUNCTION(BlueprintCallable, Category = "Climbing Ability")
	bool FindClimbingPath(FVector& OutClimbDirection);

	/** Execute drop attack on target */
	UFUNCTION(BlueprintCallable, Category = "Climbing Ability")
	void ExecuteDropAttack();

	/** Update climbing movement toward target */
	UFUNCTION(BlueprintCallable, Category = "Climbing Ability")
	void UpdateClimbingMovement(float DeltaTime);

	/** Check if target is reachable by climbing */
	UFUNCTION(BlueprintCallable, Category = "Climbing Ability")
	bool IsTargetReachableByClimbing(AActor* Target, float& OutDistance);

	/** Find nearest climbable surface */
	UFUNCTION(BlueprintCallable, Category = "Climbing Ability")
	bool FindNearestClimbableSurface(FVector& OutLocation, FVector& OutNormal);

protected:
	// ========================================
	// INTERNAL CLIMBING LOGIC
	// ========================================

	/** Evaluate if climbing is beneficial */
	bool EvaluateClimbingOpportunity();

	/** Calculate best climbing direction */
	FVector CalculateClimbingDirection();

	/** Check if path to target is blocked */
	bool IsPathToTargetBlocked();

	/** Check line of sight to target */
	bool HasLineOfSightToTarget();

	/** Calculate 3D distance to target */
	float GetDistanceToTarget() const;

	/** Check if target is below current position */
	bool IsTargetBelow() const;

	/** Perform AI climbing decision making */
	void ProcessClimbingAI(float DeltaTime);

private:
	/** Cached climbing movement component */
	UPROPERTY()
	UZombieClimbingMovementComponent* ClimbingMovement;

	/** Time since last climbing check */
	float TimeSinceLastCheck = 0.0f;

	/** Last known target location */
	FVector LastTargetLocation = FVector::ZeroVector;
};
