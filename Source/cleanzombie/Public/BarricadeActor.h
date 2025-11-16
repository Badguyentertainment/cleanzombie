// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZombieTargetInterface.h"
#include "BarricadeActor.generated.h"

/**
 * Barricade that can be attacked and destroyed by zombies
 * Example implementation of ZombieTargetInterface
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API ABarricadeActor : public AActor, public IZombieTargetInterface
{
	GENERATED_BODY()

public:
	ABarricadeActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// Damage
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// ========================================
	// BARRICADE PROPERTIES
	// ========================================

	/** Visual mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barricade")
	class UStaticMeshComponent* BarricadeMesh;

	/** Box collision */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Barricade")
	class UBoxComponent* CollisionBox;

	/** Current health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Stats", Replicated)
	float CurrentHealth = 500.0f;

	/** Maximum health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Stats")
	float MaxHealth = 500.0f;

	/** Is barricade destroyed? */
	UPROPERTY(BlueprintReadOnly, Category = "Barricade|Stats", Replicated)
	bool bIsDestroyed = false;

	/** Damage threshold to show visual damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Visuals")
	float DamageThresholdLight = 0.75f; // 75% health

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Visuals")
	float DamageThresholdMedium = 0.5f; // 50% health

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Visuals")
	float DamageThresholdHeavy = 0.25f; // 25% health

	/** Damaged materials */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Visuals")
	class UMaterialInterface* LightDamageMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Visuals")
	class UMaterialInterface* MediumDamageMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Visuals")
	class UMaterialInterface* HeavyDamageMaterial;

	/** Effect to spawn when destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Effects")
	class UParticleSystem* DestructionEffect;

	/** Sound when hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Audio")
	class USoundBase* HitSound;

	/** Sound when destroyed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Barricade|Audio")
	class USoundBase* DestroySound;

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
	// BARRICADE FUNCTIONS
	// ========================================

	/** Repair the barricade */
	UFUNCTION(BlueprintCallable, Category = "Barricade")
	void Repair(float Amount);

	/** Fully repair */
	UFUNCTION(BlueprintCallable, Category = "Barricade")
	void FullRepair();

	/** Destroy barricade */
	UFUNCTION(BlueprintCallable, Category = "Barricade")
	void DestroyBarricade();

	/** Get health percentage */
	UFUNCTION(BlueprintPure, Category = "Barricade")
	float GetHealthPercentage() const;

	/** Update visual damage state */
	UFUNCTION(BlueprintCallable, Category = "Barricade")
	void UpdateVisualDamage();

	// ========================================
	// EVENTS
	// ========================================

	/** Called when barricade takes damage */
	UFUNCTION(BlueprintNativeEvent, Category = "Barricade|Events")
	void OnBarricadeDamaged(float Damage, AActor* DamageSource);
	virtual void OnBarricadeDamaged_Implementation(float Damage, AActor* DamageSource);

	/** Called when barricade is destroyed */
	UFUNCTION(BlueprintNativeEvent, Category = "Barricade|Events")
	void OnBarricadeDestroyed();
	virtual void OnBarricadeDestroyed_Implementation();

	// Network replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Zombies currently targeting this barricade */
	UPROPERTY()
	TArray<AActor*> TargetingZombies;

	/** Original material */
	UPROPERTY()
	class UMaterialInterface* OriginalMaterial;
};
