// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "StatusEffectComponent.h"
#include "ExploderAbility.generated.h"

/**
 * Explosion state
 */
UENUM(BlueprintType)
enum class EExplosionState : uint8
{
	Idle,
	Charging,
	Detonating,
	Exploded
};

/**
 * Explosion type
 */
UENUM(BlueprintType)
enum class EExplosionType : uint8
{
	Standard,		// Normal explosion
	Fire,			// Sets targets on fire
	Acid,			// Applies acid DoT
	Poison,			// Poison cloud
	Shrapnel,		// Bleeding damage
	EMP,			// Stuns and disables
	Nuclear			// Massive damage + irradiation
};

/**
 * Exploder zombie ability - suicide bomber that detonates near targets
 * High-risk, high-reward gameplay with massive AoE damage
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UExploderAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	UExploderAbility();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// EXPLOSION ABILITY
	// ========================================

	/** Trigger the explosion */
	UFUNCTION(BlueprintCallable, Category = "Exploder")
	void Detonate();

	/** Start charging explosion */
	UFUNCTION(BlueprintCallable, Category = "Exploder")
	void StartCharging();

	/** Cancel charging */
	UFUNCTION(BlueprintCallable, Category = "Exploder")
	void CancelCharge();

	/** Is currently charging? */
	UFUNCTION(BlueprintPure, Category = "Exploder")
	bool IsCharging() const { return CurrentExplosionState == EExplosionState::Charging; }

	/** Has exploded? */
	UFUNCTION(BlueprintPure, Category = "Exploder")
	bool HasExploded() const { return CurrentExplosionState == EExplosionState::Exploded; }

	/** Get charge progress (0.0 to 1.0) */
	UFUNCTION(BlueprintPure, Category = "Exploder")
	float GetChargeProgress() const;

	/** Get explosion state */
	UFUNCTION(BlueprintPure, Category = "Exploder")
	EExplosionState GetExplosionState() const { return CurrentExplosionState; }

	// ========================================
	// CONFIGURATION
	// ========================================

	/** Explosion type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	EExplosionType ExplosionType = EExplosionType::Standard;

	/** Explosion damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	float ExplosionDamage = 200.0f;

	/** Explosion radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	float ExplosionRadius = 800.0f;

	/** Detonation proximity (auto-detonate when target this close) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	float DetonationProximity = 300.0f;

	/** Charge time before detonation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	float ChargeTime = 2.0f;

	/** Auto-detonate on death? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	bool bDetonateOnDeath = true;

	/** Kills self on explosion? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	bool bSuicideExplosion = true;

	/** Damage falloff (0.0 = no falloff, 1.0 = linear falloff) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	float DamageFalloff = 0.7f;

	/** Explosion particle effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Visuals")
	UParticleSystem* ExplosionParticle = nullptr;

	/** Explosion Niagara effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Visuals")
	class UNiagaraSystem* ExplosionNiagara = nullptr;

	/** Explosion sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Visuals")
	USoundBase* ExplosionSound = nullptr;

	/** Charging sound (looping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Visuals")
	USoundBase* ChargingSound = nullptr;

	/** Camera shake on explosion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Visuals")
	TSubclassOf<class UCameraShakeBase> ExplosionCameraShake;

	/** Camera shake radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Visuals")
	float CameraShakeRadius = 2000.0f;

	/** Status effect to apply on explosion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Effects")
	EStatusEffectType ExplosionStatusEffect = EStatusEffectType::None;

	/** Status effect strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Effects")
	float StatusEffectStrength = 1.0f;

	/** Status effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config|Effects")
	float StatusEffectDuration = 10.0f;

	/** Movement speed multiplier while charging */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	float ChargingSpeedMultiplier = 1.5f;

	/** Can be interrupted? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploder|Config")
	bool bCanBeInterrupted = false;

	// ========================================
	// EVENTS
	// ========================================

	/** Called when charging starts */
	UFUNCTION(BlueprintNativeEvent, Category = "Exploder|Events")
	void OnChargingStarted();
	virtual void OnChargingStarted_Implementation();

	/** Called when explosion triggers */
	UFUNCTION(BlueprintNativeEvent, Category = "Exploder|Events")
	void OnExplosionTriggered();
	virtual void OnExplosionTriggered_Implementation();

	/** Called when actor is damaged by explosion */
	UFUNCTION(BlueprintNativeEvent, Category = "Exploder|Events")
	void OnActorDamaged(AActor* DamagedActor, float Damage);
	virtual void OnActorDamaged_Implementation(AActor* DamagedActor, float Damage);

	/** Called when charging is cancelled */
	UFUNCTION(BlueprintNativeEvent, Category = "Exploder|Events")
	void OnChargingCancelled();
	virtual void OnChargingCancelled_Implementation();

protected:
	/** Current explosion state */
	UPROPERTY(BlueprintReadOnly, Category = "Exploder")
	EExplosionState CurrentExplosionState = EExplosionState::Idle;

	/** Charge elapsed time */
	float ChargeElapsedTime = 0.0f;

	/** Spawned charging audio */
	UPROPERTY()
	class UAudioComponent* ChargingAudioComponent = nullptr;

	/** Find targets in detonation range */
	TArray<AActor*> FindTargetsInRange();

	/** Check if should auto-detonate */
	bool ShouldAutoDetonate();

	/** Execute explosion */
	void ExecuteExplosion();

	/** Apply explosion damage to actor */
	void ApplyExplosionDamage(AActor* Target, float Distance);

	/** Apply status effects from explosion */
	void ApplyExplosionEffects(AActor* Target);

	/** Spawn visual effects */
	void SpawnExplosionEffects();

	/** Handle death event */
	UFUNCTION()
	void OnOwnerDeath(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);
};
