// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "ZombieClimbingMovementComponent.h"
#include "ClimbingAIComponent.generated.h"

/**
 * Climbing ability component for zombies
 * Provides wall and ceiling climbing capabilities with AI decision-making
 * Inherits from ZombieAbilityComponent for modular ability system
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UClimbingAIComponent : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	UClimbingAIComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// AI CONFIGURATION
	// ========================================

	/** Should AI automatically decide when to climb? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Behavior")
	bool bAutoClimbing = true;

	/** How often to check for climbing opportunities (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Behavior", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float ClimbingCheckInterval = 0.5f;

	/** Prefer climbing to reach target if path is blocked? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Behavior")
	bool bClimbWhenPathBlocked = true;

	/** Distance at which to consider climbing to reach target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Behavior", meta = (ClampMin = "0.0"))
	float ClimbingConsiderationDistance = 500.0f;

	/** Should drop from ceiling/walls to attack targets below? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Attack")
	bool bDropToAttack = true;

	/** Distance from target to drop for attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Attack", meta = (ClampMin = "0.0"))
	float DropAttackDistance = 300.0f;

	/** Vertical distance threshold for drop attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Attack", meta = (ClampMin = "0.0"))
	float DropAttackHeightMin = 100.0f;

	/** Maximum height from which to drop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Attack", meta = (ClampMin = "0.0"))
	float MaxDropHeight = 1000.0f;

	/** Prefer walls over ground when both paths available? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Behavior")
	bool bPreferClimbingPaths = false;

	/** Minimum time to spend climbing before considering dropping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Behavior", meta = (ClampMin = "0.0"))
	float MinimumClimbTime = 2.0f;

	/** Enable debug visualization for AI decisions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbing AI|Debug")
	bool bShowDebugInfo = false;

	// ========================================
	// AI STATE
	// ========================================

	/** Current AI target actor (usually a player) */
	UPROPERTY(BlueprintReadWrite, Category = "Climbing AI|State")
	AActor* CurrentTarget;

	/** Is AI currently trying to climb? */
	UPROPERTY(BlueprintReadOnly, Category = "Climbing AI|State")
	bool bWantsToClimb = false;

	/** Time spent in current climbing session */
	UPROPERTY(BlueprintReadOnly, Category = "Climbing AI|State")
	float CurrentClimbTime = 0.0f;

	// ========================================
	// AI FUNCTIONS
	// ========================================

	/** Manually set AI target */
	UFUNCTION(BlueprintCallable, Category = "Climbing AI")
	void SetTarget(AActor* NewTarget);

	/** Check if should start climbing toward target */
	UFUNCTION(BlueprintCallable, Category = "Climbing AI")
	bool ShouldClimbToTarget();

	/** Check if should drop to attack target */
	UFUNCTION(BlueprintCallable, Category = "Climbing AI")
	bool ShouldDropToAttack();

	/** Find climbing path to target */
	UFUNCTION(BlueprintCallable, Category = "Climbing AI")
	bool FindClimbingPath(FVector& OutClimbDirection);

	/** Execute drop attack on target */
	UFUNCTION(BlueprintCallable, Category = "Climbing AI")
	void ExecuteDropAttack();

	/** Update climbing movement toward target */
	UFUNCTION(BlueprintCallable, Category = "Climbing AI")
	void UpdateClimbingMovement(float DeltaTime);

	/** Check if target is reachable by climbing */
	UFUNCTION(BlueprintCallable, Category = "Climbing AI")
	bool IsTargetReachableByClimbing(AActor* Target, float& OutDistance);

	/** Find nearest climbable surface */
	UFUNCTION(BlueprintCallable, Category = "Climbing AI")
	bool FindNearestClimbableSurface(FVector& OutLocation, FVector& OutNormal);

protected:
	// ========================================
	// INTERNAL AI LOGIC
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

	/** Cached character owner */
	UPROPERTY()
	ACharacter* OwnerCharacter;

	/** Time since last climbing check */
	float TimeSinceLastCheck = 0.0f;

	/** Last known target location */
	FVector LastTargetLocation = FVector::ZeroVector;
};
