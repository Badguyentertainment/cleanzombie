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

	// DEBUFFS - Damage over Time
	Poison,			// Damage over time
	Acid,			// Armor/health debuff + DoT
	Fire,			// High DoT, can spread
	Bleeding,		// Physical DoT, gets worse with movement
	Corroded,		// Armor degradation + DoT
	Diseased,		// Slow DoT, highly contagious
	Irradiated,		// Persistent DoT, prevents healing

	// DEBUFFS - Movement/Control
	Slowing,		// Movement speed reduction
	Frozen,			// Severe movement reduction, shatters on damage
	Stun,			// Complete immobilization
	Rooted,			// Can't move but can still attack

	// DEBUFFS - Combat
	Blinding,		// Vision impairment
	Weakness,		// Damage output reduction
	Vulnerability,	// Increased damage taken
	Disarmed,		// Can't use weapons

	// DEBUFFS - Special
	Electrified,	// DoT + chance to chain to nearby actors
	Confused,		// Random movement/attacks
	Cursed,			// Multiple negative effects

	// BUFFS - Healing
	Regeneration,	// Health over time
	Shielded,		// Temporary damage absorption
	Blessed,		// Multiple positive effects

	// BUFFS - Combat
	DamageBoost,	// Increased damage output
	Haste,			// Increased movement/attack speed
	Invulnerable,	// Immune to all damage

	// BUFFS - Utility
	Invisible,		// Can't be targeted
	Fortified,		// Reduced damage taken

	Custom			// For custom Blueprint effects
};

/**
 * Severity levels for status effects
 */
UENUM(BlueprintType)
enum class EEffectSeverity : uint8
{
	Minor,		// 0.0-0.33
	Moderate,	// 0.34-0.66
	Severe,		// 0.67-0.89
	Critical	// 0.90-1.0+
};

/**
 * Visual feedback configuration for effects
 */
USTRUCT(BlueprintType)
struct FStatusEffectVisuals
{
	GENERATED_BODY()

	/** Particle system to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	UParticleSystem* ParticleEffect = nullptr;

	/** Niagara system to spawn (takes priority over ParticleEffect) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	class UNiagaraSystem* NiagaraEffect = nullptr;

	/** Sound to play when effect is applied */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	USoundBase* ApplySound = nullptr;

	/** Looping sound while effect is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	USoundBase* LoopSound = nullptr;

	/** Material parameter collection for screen effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	class UMaterialParameterCollection* ScreenEffect = nullptr;

	/** Post process material for camera effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	UMaterialInterface* PostProcessMaterial = nullptr;

	/** Color tint for this effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor EffectColor = FLinearColor::White;

	/** Socket name to attach particle to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FName AttachSocketName = NAME_None;
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

	/** Effect strength/magnitude (0.0 to 1.0+) */
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

	/** Healing per tick (for HoT effects) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	float HealingPerTick = 0.0f;

	/** Tick interval for DoT/HoT */
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

	/** Severity level */
	UPROPERTY(BlueprintReadOnly, Category = "Status Effect")
	EEffectSeverity Severity = EEffectSeverity::Moderate;

	/** Can this effect be cleansed? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	bool bCanBeCleansed = true;

	/** Is this a contagious effect? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	bool bIsContagious = false;

	/** Contagion radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	float ContagionRadius = 300.0f;

	/** Chance to spread (0.0 to 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	float ContagionChance = 0.25f;

	/** Time between contagion checks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	float ContagionTickInterval = 2.0f;

	/** Time since last contagion check */
	UPROPERTY()
	float TimeSinceLastContagion = 0.0f;

	/** Visual feedback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effect")
	FStatusEffectVisuals Visuals;

	/** Spawned particle component (runtime) */
	UPROPERTY()
	class UParticleSystemComponent* SpawnedParticle = nullptr;

	/** Spawned audio component (runtime) */
	UPROPERTY()
	class UAudioComponent* SpawnedAudio = nullptr;

	FStatusEffect()
	{
		TimeRemaining = Duration;
	}
};

/**
 * Effect combo/synergy definition
 */
USTRUCT(BlueprintType)
struct FEffectCombo
{
	GENERATED_BODY()

	/** First effect type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	EStatusEffectType EffectA = EStatusEffectType::None;

	/** Second effect type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	EStatusEffectType EffectB = EStatusEffectType::None;

	/** Resulting effect from combo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	EStatusEffectType ResultEffect = EStatusEffectType::None;

	/** Bonus damage from combo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float BonusDamage = 50.0f;

	/** Combo radius (for AoE combos) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float ComboRadius = 500.0f;

	/** Consume both effects on combo? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	bool bConsumeBothEffects = false;
};

/**
 * Resistance to specific effect types
 */
USTRUCT(BlueprintType)
struct FEffectResistance
{
	GENERATED_BODY()

	/** Effect type to resist */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
	EStatusEffectType EffectType = EStatusEffectType::None;

	/** Resistance percentage (0.0 = no resist, 1.0 = immune) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
	float ResistanceAmount = 0.0f;

	/** Reduce duration? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
	bool bReducesDuration = true;

	/** Reduce strength? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resistance")
	bool bReducesStrength = true;
};

/**
 * Component that manages status effects on an actor
 * Handles poison, slowing, blinding, burning, healing, buffs, debuffs, combos, and more
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UStatusEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStatusEffectComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	// RESISTANCE & IMMUNITY
	// ========================================

	/** Add resistance to an effect type */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Resistance")
	void AddResistance(EStatusEffectType EffectType, float ResistanceAmount, bool bReducesDuration = true, bool bReducesStrength = true);

	/** Remove resistance to an effect type */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Resistance")
	void RemoveResistance(EStatusEffectType EffectType);

	/** Get resistance to an effect type */
	UFUNCTION(BlueprintPure, Category = "Status Effects|Resistance")
	float GetResistance(EStatusEffectType EffectType) const;

	/** Check if immune to effect type */
	UFUNCTION(BlueprintPure, Category = "Status Effects|Resistance")
	bool IsImmuneTo(EStatusEffectType EffectType) const;

	// ========================================
	// CLEANSING & CURING
	// ========================================

	/** Cleanse a specific effect type */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Cleanse")
	void CleanseEffect(EStatusEffectType EffectType);

	/** Cleanse all debuffs */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Cleanse")
	void CleanseAllDebuffs();

	/** Cleanse all buffs */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Cleanse")
	void CleanseAllBuffs();

	/** Cleanse all cleansable effects */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Cleanse")
	void CleanseAll();

	/** Cleanse effects by severity */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Cleanse")
	void CleanseBySeverity(EEffectSeverity MaxSeverity);

	// ========================================
	// COMBOS & SYNERGIES
	// ========================================

	/** Check for effect combos and apply them */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Combos")
	void CheckForCombos();

	/** Manually trigger a combo between two effects */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Combos")
	bool TriggerCombo(EStatusEffectType EffectA, EStatusEffectType EffectB);

	// ========================================
	// CONTAGION & SPREADING
	// ========================================

	/** Spread contagious effects to nearby actors */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Contagion")
	void SpreadContagiousEffects();

	/** Check if this actor can spread effects */
	UFUNCTION(BlueprintPure, Category = "Status Effects|Contagion")
	bool HasContagiousEffects() const;

	// ========================================
	// BUFFS & HEALING
	// ========================================

	/** Get current shield amount */
	UFUNCTION(BlueprintPure, Category = "Status Effects|Buffs")
	float GetShieldAmount() const;

	/** Apply shield (damage absorption) */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Buffs")
	void ApplyShield(float ShieldAmount, float Duration);

	/** Absorb damage with shield */
	UFUNCTION(BlueprintCallable, Category = "Status Effects|Buffs")
	float AbsorbDamage(float IncomingDamage);

	/** Get healing multiplier from regeneration */
	UFUNCTION(BlueprintPure, Category = "Status Effects|Buffs")
	float GetHealingMultiplier() const;

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

	/** Default bleeding damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultBleedingDamage = 6.0f;

	/** Default electrified damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultElectrifiedDamage = 7.0f;

	/** Default frozen slow percentage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultFrozenSlowPercentage = 0.8f; // 80% speed reduction

	/** Default corroded damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultCorrodedDamage = 4.0f;

	/** Default diseased damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultDiseasedDamage = 3.0f;

	/** Default irradiated damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultIrradiatedDamage = 8.0f;

	/** Default regeneration healing per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config")
	float DefaultRegenerationHealing = 10.0f;

	/** Enable effect combos */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config|Combos")
	bool bEnableCombos = true;

	/** Predefined effect combos */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config|Combos")
	TArray<FEffectCombo> EffectCombos;

	/** Effect resistances */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config|Resistance")
	TArray<FEffectResistance> Resistances;

	/** Enable visual feedback */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config|Visuals")
	bool bEnableVisuals = true;

	/** Enable contagion spreading */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status Effects|Config|Contagion")
	bool bEnableContagion = true;

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

	/** Called when HoT healing is applied */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Effects|Events")
	void OnHoTHealingApplied(float Healing, EStatusEffectType EffectType);
	virtual void OnHoTHealingApplied_Implementation(float Healing, EStatusEffectType EffectType);

	/** Called when an effect combo is triggered */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Effects|Events")
	void OnComboTriggered(const FEffectCombo& Combo);
	virtual void OnComboTriggered_Implementation(const FEffectCombo& Combo);

	/** Called when an effect is cleansed */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Effects|Events")
	void OnEffectCleansed(EStatusEffectType EffectType);
	virtual void OnEffectCleansed_Implementation(EStatusEffectType EffectType);

	/** Called when effect spreads to another actor */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Effects|Events")
	void OnEffectSpread(AActor* TargetActor, EStatusEffectType EffectType);
	virtual void OnEffectSpread_Implementation(AActor* TargetActor, EStatusEffectType EffectType);

	/** Called when shield absorbs damage */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Effects|Events")
	void OnShieldAbsorbed(float DamageAbsorbed, float RemainingShield);
	virtual void OnShieldAbsorbed_Implementation(float DamageAbsorbed, float RemainingShield);

protected:
	/** Active status effects */
	UPROPERTY(BlueprintReadOnly, Category = "Status Effects")
	TArray<FStatusEffect> ActiveEffects;

	/** Current shield amount */
	UPROPERTY(BlueprintReadOnly, Category = "Status Effects")
	float CurrentShieldAmount = 0.0f;

	/** Update all active effects */
	void UpdateActiveEffects(float DeltaTime);

	/** Process a single effect */
	void ProcessEffect(FStatusEffect& Effect, float DeltaTime);

	/** Apply DoT damage from effect */
	void ApplyDoTDamage(const FStatusEffect& Effect);

	/** Apply HoT healing from effect */
	void ApplyHoTHealing(const FStatusEffect& Effect);

	/** Get default effect parameters */
	FStatusEffect CreateDefaultEffect(EStatusEffectType EffectType, float Strength, float Duration, AActor* Instigator);

	/** Apply effect resistance */
	void ApplyResistance(FStatusEffect& Effect) const;

	/** Calculate severity based on strength */
	EEffectSeverity CalculateSeverity(float Strength) const;

	/** Check if effect is a debuff */
	bool IsDebuff(EStatusEffectType EffectType) const;

	/** Check if effect is a buff */
	bool IsBuff(EStatusEffectType EffectType) const;

	/** Spawn visual feedback for effect */
	void SpawnVisualFeedback(FStatusEffect& Effect);

	/** Cleanup visual feedback for effect */
	void CleanupVisualFeedback(FStatusEffect& Effect);

	/** Setup default combos in constructor */
	void SetupDefaultCombos();

	/** Find combo between two effects */
	FEffectCombo* FindCombo(EStatusEffectType EffectA, EStatusEffectType EffectB);
};
