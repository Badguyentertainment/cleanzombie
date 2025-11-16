// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZombieTargetInterface.generated.h"

/**
 * Target type enum for zombie targeting
 */
UENUM(BlueprintType)
enum class EZombieTargetType : uint8
{
	None UMETA(DisplayName = "None"),
	Player UMETA(DisplayName = "Player"),
	NPC UMETA(DisplayName = "NPC/Marine"),
	Barricade UMETA(DisplayName = "Barricade"),
	DestructibleObject UMETA(DisplayName = "Destructible Object"),
	Vehicle UMETA(DisplayName = "Vehicle"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Target priority level
 */
UENUM(BlueprintType)
enum class ETargetPriority : uint8
{
	VeryLow UMETA(DisplayName = "Very Low"),
	Low UMETA(DisplayName = "Low"),
	Medium UMETA(DisplayName = "Medium"),
	High UMETA(DisplayName = "High"),
	Critical UMETA(DisplayName = "Critical")
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UZombieTargetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for all objects that can be targeted and attacked by zombies
 * Implement this on: Players, NPCs, Barricades, Destructible Objects, etc.
 */
class CLEANZOMBIE_API IZombieTargetInterface
{
	GENERATED_BODY()

public:
	/**
	 * Can this target be attacked by zombies?
	 * @param AttackingZombie - The zombie attempting to target this
	 * @return true if targetable
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	bool CanBeTargeted(AActor* AttackingZombie) const;
	virtual bool CanBeTargeted_Implementation(AActor* AttackingZombie) const { return true; }

	/**
	 * Get the type of this target
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	EZombieTargetType GetTargetType() const;
	virtual EZombieTargetType GetTargetType_Implementation() const { return EZombieTargetType::Custom; }

	/**
	 * Get the base priority of this target
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	ETargetPriority GetTargetPriority() const;
	virtual ETargetPriority GetTargetPriority_Implementation() const { return ETargetPriority::Medium; }

	/**
	 * Get target location for zombies to move toward
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	FVector GetTargetLocation() const;
	virtual FVector GetTargetLocation_Implementation() const { return FVector::ZeroVector; }

	/**
	 * Get current health of this target
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	float GetCurrentHealth() const;
	virtual float GetCurrentHealth_Implementation() const { return 100.0f; }

	/**
	 * Get maximum health of this target
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	float GetMaxHealth() const;
	virtual float GetMaxHealth_Implementation() const { return 100.0f; }

	/**
	 * Is this target alive/active?
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	bool IsTargetAlive() const;
	virtual bool IsTargetAlive_Implementation() const { return true; }

	/**
	 * Should this target be visible to AI perception?
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	bool IsVisibleToZombies() const;
	virtual bool IsVisibleToZombies_Implementation() const { return true; }

	/**
	 * Called when a zombie starts targeting this object
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	void OnTargetedByZombie(AActor* Zombie);
	virtual void OnTargetedByZombie_Implementation(AActor* Zombie) {}

	/**
	 * Called when a zombie stops targeting this object
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	void OnUnTargetedByZombie(AActor* Zombie);
	virtual void OnUnTargetedByZombie_Implementation(AActor* Zombie) {}

	/**
	 * Called when damaged by a zombie
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	void OnDamagedByZombie(float Damage, AActor* Zombie);
	virtual void OnDamagedByZombie_Implementation(float Damage, AActor* Zombie) {}

	/**
	 * Called when destroyed by zombies
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	void OnDestroyedByZombies();
	virtual void OnDestroyedByZombies_Implementation() {}

	/**
	 * Get priority modifier based on context (distance, health, etc.)
	 * Higher values = higher priority
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	float GetDynamicPriorityModifier(AActor* EvaluatingZombie) const;
	virtual float GetDynamicPriorityModifier_Implementation(AActor* EvaluatingZombie) const { return 1.0f; }

	/**
	 * How many zombies are currently targeting this?
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	int32 GetZombieTargeterCount() const;
	virtual int32 GetZombieTargeterCount_Implementation() const { return 0; }

	/**
	 * Can multiple zombies target this simultaneously?
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	bool AllowsMultipleTargeters() const;
	virtual bool AllowsMultipleTargeters_Implementation() const { return true; }

	/**
	 * Get recommended attack range for this target
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Target")
	float GetAttackRange() const;
	virtual float GetAttackRange_Implementation() const { return 150.0f; }
};
