// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "ZombieTargetInterface.h"
#include "MultiTargetingAbility.generated.h"

/**
 * Target evaluation data
 */
USTRUCT(BlueprintType)
struct FTargetEvaluationData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Target Evaluation")
	AActor* Target = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Target Evaluation")
	EZombieTargetType TargetType = EZombieTargetType::None;

	UPROPERTY(BlueprintReadWrite, Category = "Target Evaluation")
	ETargetPriority BasePriority = ETargetPriority::Medium;

	UPROPERTY(BlueprintReadWrite, Category = "Target Evaluation")
	float Distance = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Target Evaluation")
	float FinalScore = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Target Evaluation")
	bool bHasLineOfSight = false;

	UPROPERTY(BlueprintReadWrite, Category = "Target Evaluation")
	float HealthPercentage = 1.0f;
};

/**
 * Multi-targeting ability for zombies
 * Enables zombies to detect and attack players, NPCs, barricades, and objects
 * Integrates with modular ability framework
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UMultiTargetingAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	UMultiTargetingAbility();

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

	// ========================================
	// DETECTION CONFIGURATION
	// ========================================

	/** Detection range for targets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Detection", meta = (ClampMin = "0.0"))
	float DetectionRange = 2000.0f;

	/** How often to scan for targets (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Detection", meta = (ClampMin = "0.1"))
	float ScanInterval = 0.5f;

	/** Require line of sight to target? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Detection")
	bool bRequireLineOfSight = true;

	/** Sight trace channel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Detection")
	TEnumAsByte<ECollisionChannel> SightTraceChannel = ECC_Visibility;

	/** Use AI perception system (more expensive but better) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Detection")
	bool bUseAIPerception = false;

	// ========================================
	// PRIORITY CONFIGURATION
	// ========================================

	/** Base priority scores for each target type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Priority")
	TMap<EZombieTargetType, float> TargetTypePriorities = {
		{EZombieTargetType::Player, 100.0f},
		{EZombieTargetType::NPC, 80.0f},
		{EZombieTargetType::Barricade, 40.0f},
		{EZombieTargetType::DestructibleObject, 20.0f},
		{EZombieTargetType::Vehicle, 30.0f}
	};

	/** Priority multipliers for different priority levels */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Priority")
	TMap<ETargetPriority, float> PriorityLevelMultipliers = {
		{ETargetPriority::VeryLow, 0.5f},
		{ETargetPriority::Low, 0.75f},
		{ETargetPriority::Medium, 1.0f},
		{ETargetPriority::High, 1.5f},
		{ETargetPriority::Critical, 2.0f}
	};

	/** Distance weighting (closer = higher priority) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Priority", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DistanceWeight = 0.3f;

	/** Health weighting (lower health = higher priority for NPCs/Players) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Priority", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HealthWeight = 0.2f;

	/** Line of sight bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Priority")
	float LineOfSightBonus = 50.0f;

	// ========================================
	// TARGET SWITCHING
	// ========================================

	/** How much better new target must be to switch (prevents ping-ponging) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Switching", meta = (ClampMin = "0.0"))
	float SwitchThreshold = 20.0f;

	/** Minimum time before considering target switch (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Switching", meta = (ClampMin = "0.0"))
	float MinTargetLockTime = 2.0f;

	/** Switch to player immediately if spotted? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Switching")
	bool bAlwaysSwitchToPlayers = true;

	/** Maximum distance to chase target before giving up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Switching", meta = (ClampMin = "0.0"))
	float MaxChaseDistance = 3000.0f;

	// ========================================
	// GROUP COORDINATION
	// ========================================

	/** Enable group coordination for barricade attacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Coordination")
	bool bEnableGroupCoordination = true;

	/** Reduce priority if too many zombies already targeting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Coordination")
	float OvercrowdingPenalty = 10.0f;

	/** Maximum zombies that should target one barricade */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Coordination")
	int32 MaxZombiesPerBarricade = 5;

	/** Prefer less-crowded targets? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Coordination")
	bool bPreferLessCrowdedTargets = true;

	// ========================================
	// TARGET FILTERING
	// ========================================

	/** Target types this zombie can attack */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Filtering")
	TArray<EZombieTargetType> AllowedTargetTypes = {
		EZombieTargetType::Player,
		EZombieTargetType::NPC,
		EZombieTargetType::Barricade,
		EZombieTargetType::DestructibleObject
	};

	/** Ignore targets with these tags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Filtering")
	TArray<FName> IgnoreTags;

	/** Only target actors with these tags (empty = no restriction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi-Targeting|Filtering")
	TArray<FName> RequiredTags;

	// ========================================
	// TARGET MANAGEMENT
	// ========================================

	/** All currently detected targets */
	UPROPERTY(BlueprintReadOnly, Category = "Multi-Targeting|State")
	TArray<FTargetEvaluationData> DetectedTargets;

	/** Current best target */
	UPROPERTY(BlueprintReadOnly, Category = "Multi-Targeting|State")
	AActor* BestTarget = nullptr;

	/** Previous target (for switch tracking) */
	UPROPERTY(BlueprintReadOnly, Category = "Multi-Targeting|State")
	AActor* PreviousTarget = nullptr;

	/** Time since last target switch */
	UPROPERTY(BlueprintReadOnly, Category = "Multi-Targeting|State")
	float TimeSinceLastSwitch = 0.0f;

	// ========================================
	// TARGETING FUNCTIONS
	// ========================================

	/**
	 * Scan for all valid targets in range
	 */
	UFUNCTION(BlueprintCallable, Category = "Multi-Targeting")
	void ScanForTargets();

	/**
	 * Evaluate a potential target and return score
	 */
	UFUNCTION(BlueprintCallable, Category = "Multi-Targeting")
	FTargetEvaluationData EvaluateTarget(AActor* PotentialTarget);

	/**
	 * Calculate priority score for a target
	 */
	UFUNCTION(BlueprintPure, Category = "Multi-Targeting")
	float CalculateTargetScore(const FTargetEvaluationData& TargetData) const;

	/**
	 * Select best target from detected targets
	 */
	UFUNCTION(BlueprintCallable, Category = "Multi-Targeting")
	AActor* SelectBestTarget();

	/**
	 * Check if should switch to a new target
	 */
	UFUNCTION(BlueprintPure, Category = "Multi-Targeting")
	bool ShouldSwitchTarget(AActor* NewTarget, float NewScore, float CurrentScore) const;

	/**
	 * Force target switch
	 */
	UFUNCTION(BlueprintCallable, Category = "Multi-Targeting")
	void SwitchTarget(AActor* NewTarget);

	/**
	 * Clear current target
	 */
	UFUNCTION(BlueprintCallable, Category = "Multi-Targeting")
	void ClearCurrentTarget();

	/**
	 * Check if actor implements target interface
	 */
	UFUNCTION(BlueprintPure, Category = "Multi-Targeting")
	static bool IsValidTarget(AActor* Actor);

	/**
	 * Get target interface from actor
	 */
	UFUNCTION(BlueprintPure, Category = "Multi-Targeting")
	static TScriptInterface<IZombieTargetInterface> GetTargetInterface(AActor* Actor);

	/**
	 * Check line of sight to target
	 */
	UFUNCTION(BlueprintCallable, Category = "Multi-Targeting")
	bool HasLineOfSight(AActor* Target) const;

	/**
	 * Get distance to target
	 */
	UFUNCTION(BlueprintPure, Category = "Multi-Targeting")
	float GetDistanceToTarget(AActor* Target) const;

protected:
	/** Time since last target scan */
	float TimeSinceLastScan = 0.0f;

	/** Cached perception component */
	UPROPERTY()
	class UAIPerceptionComponent* PerceptionComponent;

	/** Process target detection results */
	void ProcessDetectedTargets();

	/** Apply group coordination penalties */
	void ApplyCoordinationPenalties();

	/** Filter target based on allowed types and tags */
	bool PassesFilters(AActor* Target, EZombieTargetType TargetType) const;

	/** Get priority score for target type */
	float GetTypePriorityScore(EZombieTargetType TargetType) const;

	/** Get priority multiplier for priority level */
	float GetPriorityLevelMultiplier(ETargetPriority Priority) const;
};
