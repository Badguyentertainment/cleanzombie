// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "ScreamerAbility.generated.h"

/**
 * Screamer zombie ability - sonic scream stuns and disorients
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UScreamerAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	UScreamerAbility();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Execute scream */
	UFUNCTION(BlueprintCallable, Category = "Screamer")
	void ExecuteScream();

	/** Can scream? */
	UFUNCTION(BlueprintPure, Category = "Screamer")
	bool CanScream() const;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config")
	float ScreamRadius = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config")
	float StunDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config")
	float ScreamDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config")
	float ScreamCooldown = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config")
	bool bCausesBlinding = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config")
	bool bCausesConfusion = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config")
	float WindupTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config|Visuals")
	USoundBase* ScreamSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screamer|Config|Visuals")
	UParticleSystem* ScreamParticle = nullptr;

	// Events
	UFUNCTION(BlueprintNativeEvent, Category = "Screamer|Events")
	void OnScreamStarted();

	UFUNCTION(BlueprintNativeEvent, Category = "Screamer|Events")
	void OnActorAffected(AActor* Target);

protected:
	float LastScreamTime = -999.0f;
	bool bIsScreaming = false;
	float WindupElapsedTime = 0.0f;

	void ApplyScreamEffects();

	virtual void OnScreamStarted_Implementation();
	virtual void OnActorAffected_Implementation(AActor* Target);
};
