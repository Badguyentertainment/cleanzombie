// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "SpitterAbility.generated.h"

/**
 * Example ability: Spitter - Ranged acid projectile attack
 * Demonstrates how to create a new ability using the framework
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API USpitterAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	USpitterAbility();

	// ========================================
	// ABILITY OVERRIDES
	// ========================================

	virtual void InitializeAbility_Implementation() override;
	virtual bool ActivateAbility_Implementation() override;
	virtual void DeactivateAbility_Implementation() override;
	virtual void UpdateAbility_Implementation(float DeltaTime) override;
	virtual bool CanActivate_Implementation() const override;

	// ========================================
	// SPITTER CONFIGURATION
	// ========================================

	/** Projectile class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spitter Ability")
	TSubclassOf<AActor> ProjectileClass;

	/** Minimum range to spit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spitter Ability")
	float MinSpitRange = 300.0f;

	/** Maximum range to spit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spitter Ability")
	float MaxSpitRange = 1500.0f;

	/** Time between spits (cooldown) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spitter Ability")
	float SpitCooldown = 3.0f;

	/** Damage dealt by projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spitter Ability")
	float ProjectileDamage = 25.0f;

	/** Projectile speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spitter Ability")
	float ProjectileSpeed = 1000.0f;

	/** Should aim ahead of moving targets? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spitter Ability")
	bool bPredictTargetMovement = true;

	// ========================================
	// SPITTER FUNCTIONS
	// ========================================

	/** Fire a projectile at target */
	UFUNCTION(BlueprintCallable, Category = "Spitter Ability")
	void SpitAtTarget();

	/** Check if can spit at target */
	UFUNCTION(BlueprintPure, Category = "Spitter Ability")
	bool CanSpitAtTarget() const;

	/** Calculate spit direction with prediction */
	UFUNCTION(BlueprintPure, Category = "Spitter Ability")
	FVector CalculateSpitDirection() const;

protected:
	/** Time since last spit */
	float TimeSinceLastSpit = 0.0f;

	/** Socket name for projectile spawn */
	FName SpitSocketName = TEXT("MouthSocket");
};
