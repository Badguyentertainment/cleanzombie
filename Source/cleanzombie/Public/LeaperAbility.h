// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "LeaperAbility.generated.h"

/**
 * Leap attack state
 */
UENUM(BlueprintType)
enum class ELeapState : uint8
{
	Idle,
	Targeting,
	Leaping,
	Pinning,
	Recovering
};

/**
 * Leaper zombie ability - pounces on targets and pins them down
 * Perfect for ambush tactics and overwhelming single targets
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API ULeaperAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	ULeaperAbility();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// LEAP ABILITY
	// ========================================

	/** Execute the leap attack */
	UFUNCTION(BlueprintCallable, Category = "Leaper")
	void ExecuteLeap();

	/** Can we leap right now? */
	UFUNCTION(BlueprintPure, Category = "Leaper")
	bool CanLeap() const;

	/** Is currently leaping? */
	UFUNCTION(BlueprintPure, Category = "Leaper")
	bool IsLeaping() const;

	/** Is currently pinning a target? */
	UFUNCTION(BlueprintPure, Category = "Leaper")
	bool IsPinning() const;

	/** Get current leap state */
	UFUNCTION(BlueprintPure, Category = "Leaper")
	ELeapState GetLeapState() const { return CurrentLeapState; }

	/** Get pinned target */
	UFUNCTION(BlueprintPure, Category = "Leaper")
	AActor* GetPinnedTarget() const { return PinnedTarget; }

	/** Break free from pin (called by target) */
	UFUNCTION(BlueprintCallable, Category = "Leaper")
	void BreakFree(float Force);

	// ========================================
	// CONFIGURATION
	// ========================================

	/** Maximum leap range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float MaxLeapRange = 1500.0f;

	/** Minimum leap range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float MinLeapRange = 300.0f;

	/** Leap speed (units per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float LeapSpeed = 2000.0f;

	/** Leap arc height multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float LeapArcHeight = 0.5f;

	/** Damage dealt on impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float ImpactDamage = 50.0f;

	/** Damage per second while pinning */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float PinDamagePerSecond = 20.0f;

	/** Maximum pin duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float MaxPinDuration = 5.0f;

	/** Force required to break free */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float BreakFreeThreshold = 100.0f;

	/** Cooldown between leaps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float LeapCooldown = 8.0f;

	/** Detection radius for auto-targeting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float DetectionRadius = 2000.0f;

	/** Angle of leap arc (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float LeapAngle = 45.0f;

	/** Can leap through air? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	bool bCanAerialLeap = true;

	/** Stun duration on impact */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Leaper|Config")
	float StunDuration = 1.0f;

	// ========================================
	// EVENTS
	// ========================================

	/** Called when leap starts */
	UFUNCTION(BlueprintNativeEvent, Category = "Leaper|Events")
	void OnLeapStarted(AActor* Target);
	virtual void OnLeapStarted_Implementation(AActor* Target);

	/** Called when landing on target */
	UFUNCTION(BlueprintNativeEvent, Category = "Leaper|Events")
	void OnLeapImpact(AActor* Target, float Damage);
	virtual void OnLeapImpact_Implementation(AActor* Target, float Damage);

	/** Called when pinning target */
	UFUNCTION(BlueprintNativeEvent, Category = "Leaper|Events")
	void OnPinStarted(AActor* Target);
	virtual void OnPinStarted_Implementation(AActor* Target);

	/** Called when pin ends */
	UFUNCTION(BlueprintNativeEvent, Category = "Leaper|Events")
	void OnPinEnded(AActor* Target, bool bBrokenFree);
	virtual void OnPinEnded_Implementation(AActor* Target, bool bBrokenFree);

	/** Called when leap misses */
	UFUNCTION(BlueprintNativeEvent, Category = "Leaper|Events")
	void OnLeapMissed();
	virtual void OnLeapMissed_Implementation();

protected:
	/** Current leap state */
	UPROPERTY(BlueprintReadOnly, Category = "Leaper")
	ELeapState CurrentLeapState = ELeapState::Idle;

	/** Current leap target */
	UPROPERTY(BlueprintReadOnly, Category = "Leaper")
	AActor* LeapTarget = nullptr;

	/** Pinned target */
	UPROPERTY(BlueprintReadOnly, Category = "Leaper")
	AActor* PinnedTarget = nullptr;

	/** Time since leap started */
	UPROPERTY(BlueprintReadOnly, Category = "Leaper")
	float LeapElapsedTime = 0.0f;

	/** Total leap duration */
	UPROPERTY(BlueprintReadOnly, Category = "Leaper")
	float LeapDuration = 0.0f;

	/** Leap start location */
	FVector LeapStartLocation;

	/** Leap target location */
	FVector LeapTargetLocation;

	/** Accumulated break free force */
	float AccumulatedBreakForce = 0.0f;

	/** Time pinning current target */
	float PinElapsedTime = 0.0f;

	/** Last leap time */
	float LastLeapTime = -999.0f;

	/** Find best target for leap */
	AActor* FindLeapTarget();

	/** Calculate leap trajectory */
	bool CalculateLeapTrajectory(AActor* Target, FVector& OutVelocity);

	/** Update leap movement */
	void UpdateLeapMovement(float DeltaTime);

	/** Check for leap collision */
	bool CheckLeapCollision();

	/** Start pinning target */
	void StartPinning(AActor* Target);

	/** End pinning */
	void EndPinning(bool bBrokenFree);

	/** Update pin damage */
	void UpdatePinDamage(float DeltaTime);

	/** Apply leap damage */
	void ApplyLeapDamage(AActor* Target);
};
