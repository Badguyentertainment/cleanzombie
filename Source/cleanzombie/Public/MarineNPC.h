// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZombieTargetInterface.h"
#include "MarineNPC.generated.h"

/**
 * Example Marine NPC that can be targeted by zombies
 * Shows how to make NPCs targetable using the interface
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API AMarineNPC : public ACharacter, public IZombieTargetInterface
{
	GENERATED_BODY()

public:
	AMarineNPC();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Damage
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// ========================================
	// NPC PROPERTIES
	// ========================================

	/** Current health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marine|Stats", Replicated)
	float CurrentHealth = 100.0f;

	/** Maximum health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Marine|Stats")
	float MaxHealth = 100.0f;

	/** Is marine alive? */
	UPROPERTY(BlueprintReadOnly, Category = "Marine|Stats", Replicated)
	bool bIsAlive = true;

	/** Is marine wounded? (< 50% health) */
	UPROPERTY(BlueprintReadOnly, Category = "Marine|Stats")
	bool bIsWounded = false;

	// ========================================
	// TARGET INTERFACE IMPLEMENTATION
	// ========================================

	virtual bool CanBeTargeted_Implementation(AActor* AttackingZombie) const override;
	virtual EZombieTargetType GetTargetType_Implementation() const override;
	virtual ETargetPriority GetTargetPriority_Implementation() const override;
	virtual FVector GetTargetLocation_Implementation() const override;
	virtual float GetCurrentHealth_Implementation() const override;
	virtual float GetMaxHealth_Implementation() const override;
	virtual bool IsTargetAlive_Implementation() const override;
	virtual bool IsVisibleToZombies_Implementation() const override;
	virtual void OnTargetedByZombie_Implementation(AActor* Zombie) override;
	virtual void OnUnTargetedByZombie_Implementation(AActor* Zombie) override;
	virtual void OnDamagedByZombie_Implementation(float Damage, AActor* Zombie) override;
	virtual void OnDestroyedByZombies_Implementation() override;
	virtual float GetDynamicPriorityModifier_Implementation(AActor* EvaluatingZombie) const override;
	virtual int32 GetZombieTargeterCount_Implementation() const override;
	virtual bool AllowsMultipleTargeters_Implementation() const override;
	virtual float GetAttackRange_Implementation() const override;

	// ========================================
	// NPC FUNCTIONS
	// ========================================

	/** Kill the marine */
	UFUNCTION(BlueprintCallable, Category = "Marine")
	void Die(AActor* Killer);

	/** Heal the marine */
	UFUNCTION(BlueprintCallable, Category = "Marine")
	void Heal(float Amount);

	/** Get health percentage */
	UFUNCTION(BlueprintPure, Category = "Marine")
	float GetHealthPercentage() const;

	// Network replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Zombies currently targeting this marine */
	UPROPERTY()
	TArray<AActor*> TargetingZombies;
};
