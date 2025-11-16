// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatusEffectComponent.generated.h"

/**
 * Types of status effects that can be applied
 */
UENUM(BlueprintType)
enum class EStatusEffectType : uint8
{
	None,
	Poison,			// Damage over time
	Acid,			// Armor/health debuff + DoT
	Slowing,		// Movement speed reduction
	Blinding,		// Vision impairment
	Fire,			// High DoT
	Stun,			// Complete immobilization
	Weakness,		// Damage output reduction
	Custom			// For custom Blueprint effects
};

/**
 * Individual status effect instance
 */
USTRUCT(BlueprintType)
struct FStatusEffect
{
	GENERATED_BODY()

	/** Type of effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	EStatusEffectType EffectType = EStatusEffectType::None;

	/** Effect strength/magnitude */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	float Strength = 1.0f;

	/** Effect duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	float Duration = 5.0f;

	/** Time remaining */
	UPROPERTY(BlueprintReadOnly, Category = "Status Effect")
	float TimeRemaining = 0.0f;

	/** Damage per tick (for DoT effects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	float DamagePerTick = 0.0f;

	/** Tick interval for DoT */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	float TickInterval = 1.0f;

	/** Time since last tick */
	UPROPERTY()
	float TimeSinceLastTick = 0.0f;

	/** Custom tag for identification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	FName EffectTag;

	/** Who applied this effect */
	UPROPERTY(BlueprintReadOnly, Category = "Status Effect")
	AActor* Instigator = nullptr;

	/** Can this effect stack? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	bool bCanStack = false;

	/** Maximum stacks allowed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	int32 MaxStacks = 1;

	/** Current stack count */
	UPROPERTY(BlueprintReadOnly, Category = "Status Effect")
	int32 CurrentStacks = 1;

	FStatusEffect()
	{
		TimeRemaining = Duration;
	}
};

/**
 * Component that manages status effects on an actor
 * Handles poison, slowing, blinding, burning, etc.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// STATUS EFFECT MANAGEMENT
	// ========================================

	/** Apply a status effect to this actor */
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void ApplyStatusEffect(EStatusEffectType EffectType, float Strength, float Duration, AActor* Instigator);

	/** Apply a custom status effect */
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void ApplyCustomStatusEffect(const FStatusEffect& Effect);

	/** Remove a specific effect type */
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void RemoveStatusEffect(EStatusEffectType EffectType);

	/** Remove all status effects */
	UFUNCTION(BlueprintCallable, Category = "Status Effects")
	void ClearAllStatusEffects();

	/** Check if actor has a specific effect */
	UFUNCTION(BlueprintPure, Category = "Status Effects")
	bool HasStatusEffect(EStatusEffectType EffectType) const;

	/** Get active effect of type */
	UFUNCTION(BlueprintPure, Category = "Status Effects")
	FStatusEffect GetStatusEffect(EStatusEffectType EffectType) const;

	/** Get all active effects */
	UFUNCTION(BlueprintPure, Category = "Status Effects")
	TArray<FStatusEffect> GetAllActiveEffects() const { return ActiveEffects; }

	/** Get movement speed multiplier from active effects */
	UFUNCTION(BlueprintPure, Category = "Status Effects")
	float GetMovementSpeedMultiplier() const;

	/** Get damage output multiplier from active effects */
	UFUNCTION(BlueprintPure, Category = "Status Effects")
	float GetDamageOutputMultiplier() const;

	/** Is actor currently stunned? */
	UFUNCTION(BlueprintPure, Category = "Status Effects")
	bool IsStunned() const;

	/** Is actor currently blinded? */
	UFUNCTION(BlueprintPure, Category = "Status Effects")
	bool IsBlinded() const;

	// ========================================
	// CONFIGURATION
	// ========================================

	/** Default poison damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultPoisonDamage = 5.0f;

	/** Default acid damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultAcidDamage = 8.0f;

	/** Default fire damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultFireDamage = 10.0f;

	/** Default slow percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultSlowPercentage = 0.5f; // 50% speed reduction

	/** Default blind strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultBlindStrength = 0.7f; // 70% vision reduction

	/** Enable debug logging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Debug")
	bool bShowDebug = false;

	// ========================================
	// EVENTS
	// ========================================

	/** Called when a status effect is applied */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Effects|Events")
	void OnStatusEffectApplied(const FStatusEffect& Effect);
	virtual void OnStatusEffectApplied_Implementation(const FStatusEffect& Effect);

	/** Called when a status effect is removed */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Effects|Events")
	void OnStatusEffectRemoved(EStatusEffectType EffectType);
	virtual void OnStatusEffectRemoved_Implementation(EStatusEffectType EffectType);

	/** Called when DoT damage is dealt */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Effects|Events")
	void OnDoTDamageDealt(float Damage, EStatusEffectType EffectType, AActor* Instigator);
	virtual void OnDoTDamageDealt_Implementation(float Damage, EStatusEffectType EffectType, AActor* Instigator);

protected:
	/** Active status effects */
	UPROPERTY(BlueprintReadOnly, Category = "Status Effects")
	TArray<FStatusEffect> ActiveEffects;

	/** Update all active effects */
	void UpdateActiveEffects(float DeltaTime);

	/** Process a single effect */
	void ProcessEffect(FStatusEffect& Effect, float DeltaTime);

	/** Apply DoT damage from effect */
	void ApplyDoTDamage(const FStatusEffect& Effect);

	/** Get default effect parameters */
	FStatusEffect CreateDefaultEffect(EStatusEffectType EffectType, float Strength, float Duration, AActor* Instigator);
};
