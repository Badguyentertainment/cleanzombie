// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "ZombieBase.generated.h"

// Forward declarations
class UZombieAbilityComponent;
class UZombieDataAsset;

/**
 * Zombie variant type enum
 */
UENUM(BlueprintType)
enum class EZombieVariant : uint8
{
	Basic UMETA(DisplayName = "Basic Zombie"),
	Crawler UMETA(DisplayName = "Crawler"),
	Climber UMETA(DisplayName = "Wall Climber"),
	Spitter UMETA(DisplayName = "Acid Spitter"),
	Tunneler UMETA(DisplayName = "Tunneler"),
	Tank UMETA(DisplayName = "Tank"),
	Runner UMETA(DisplayName = "Fast Runner"),
	Exploder UMETA(DisplayName = "Exploder"),
	Custom UMETA(DisplayName = "Custom Variant")
};

/**
 * Zombie configuration data structure
 */
USTRUCT(BlueprintType)
struct FZombieConfigData : public FTableRowBase
{
	GENERATED_BODY()

	/** Variant type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EZombieVariant VariantType = EZombieVariant::Basic;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FText DisplayName;

	/** Health points */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Stats")
	float MaxHealth = 100.0f;

	/** Movement speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Stats")
	float MovementSpeed = 300.0f;

	/** Attack damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Stats")
	float AttackDamage = 20.0f;

	/** Attack range */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Stats")
	float AttackRange = 150.0f;

	/** Attack rate (attacks per second) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Stats")
	float AttackRate = 1.0f;

	/** Detection range for players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|AI")
	float DetectionRange = 1500.0f;

	/** Ability components to add */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Abilities")
	TArray<TSubclassOf<UZombieAbilityComponent>> AbilityClasses;

	/** Skeletal mesh to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visuals")
	class USkeletalMesh* ZombieMesh;

	/** Animation Blueprint to use */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visuals")
	TSubclassOf<class UAnimInstance> AnimationBlueprint;

	/** Scale multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visuals")
	FVector ScaleMultiplier = FVector(1.0f, 1.0f, 1.0f);

	/** Point value when killed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Gameplay")
	int32 PointValue = 50;

	/** Spawn weight (for random spawning) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Gameplay")
	float SpawnWeight = 1.0f;

	FZombieConfigData()
	{
		DisplayName = FText::FromString(TEXT("Basic Zombie"));
	}
};

/**
 * Base zombie class with modular ability system
 * All zombie variants inherit from this
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API AZombieBase : public ACharacter
{
	GENERATED_BODY()

public:
	AZombieBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// ZOMBIE CONFIGURATION
	// ========================================

	/** Zombie variant type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Config", Replicated)
	EZombieVariant ZombieVariant = EZombieVariant::Basic;

	/** Configuration data table row name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Config")
	FName ConfigRowName;

	/** Reference to configuration data table */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Config")
	class UDataTable* ZombieConfigTable;

	/** Initialize zombie from configuration */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Config")
	void InitializeFromConfig(FName RowName);

	/** Initialize zombie from variant type */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Config")
	void InitializeFromVariant(EZombieVariant Variant);

	/** Get current configuration */
	UFUNCTION(BlueprintPure, Category = "Zombie|Config")
	FZombieConfigData GetCurrentConfig() const { return CurrentConfig; }

	// ========================================
	// ZOMBIE STATS
	// ========================================

	/** Current health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Stats", Replicated)
	float CurrentHealth = 100.0f;

	/** Maximum health */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Stats", Replicated)
	float MaxHealth = 100.0f;

	/** Is zombie alive? */
	UPROPERTY(BlueprintReadOnly, Category = "Zombie|Stats", Replicated)
	bool bIsAlive = true;

	/** Current target actor */
	UPROPERTY(BlueprintReadWrite, Category = "Zombie|Targeting", Replicated)
	AActor* CurrentTarget;

	/** Take damage */
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	/** Kill this zombie */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Stats")
	virtual void Die(AActor* Killer);

	/** Heal zombie */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Stats")
	void Heal(float Amount);

	// ========================================
	// ABILITY SYSTEM
	// ========================================

	/** All ability components on this zombie */
	UPROPERTY(BlueprintReadOnly, Category = "Zombie|Abilities")
	TArray<UZombieAbilityComponent*> AbilityComponents;

	/** Add an ability component */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Abilities")
	UZombieAbilityComponent* AddAbility(TSubclassOf<UZombieAbilityComponent> AbilityClass);

	/** Remove an ability component */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Abilities")
	void RemoveAbility(UZombieAbilityComponent* Ability);

	/** Get ability by class */
	UFUNCTION(BlueprintPure, Category = "Zombie|Abilities")
	UZombieAbilityComponent* GetAbilityByClass(TSubclassOf<UZombieAbilityComponent> AbilityClass) const;

	/** Get all abilities of a specific class */
	UFUNCTION(BlueprintPure, Category = "Zombie|Abilities")
	TArray<UZombieAbilityComponent*> GetAbilitiesByClass(TSubclassOf<UZombieAbilityComponent> AbilityClass) const;

	/** Check if has ability */
	UFUNCTION(BlueprintPure, Category = "Zombie|Abilities")
	bool HasAbility(TSubclassOf<UZombieAbilityComponent> AbilityClass) const;

	/** Activate an ability */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Abilities")
	bool ActivateAbility(TSubclassOf<UZombieAbilityComponent> AbilityClass);

	/** Deactivate an ability */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Abilities")
	void DeactivateAbility(TSubclassOf<UZombieAbilityComponent> AbilityClass);

	/** Get all active abilities */
	UFUNCTION(BlueprintPure, Category = "Zombie|Abilities")
	TArray<UZombieAbilityComponent*> GetActiveAbilities() const;

	// ========================================
	// TARGETING
	// ========================================

	/** Set current target */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Targeting")
	virtual void SetTarget(AActor* NewTarget);

	/** Clear current target */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Targeting")
	void ClearTarget();

	/** Get current target */
	UFUNCTION(BlueprintPure, Category = "Zombie|Targeting")
	AActor* GetTarget() const { return CurrentTarget; }

	/** Check if has valid target */
	UFUNCTION(BlueprintPure, Category = "Zombie|Targeting")
	bool HasValidTarget() const;

	// ========================================
	// EVENTS
	// ========================================

	/** Called when zombie takes damage */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie|Events")
	void OnDamaged(float Damage, AActor* DamageSource);
	virtual void OnDamaged_Implementation(float Damage, AActor* DamageSource);

	/** Called when zombie dies */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie|Events")
	void OnDeath(AActor* Killer);
	virtual void OnDeath_Implementation(AActor* Killer);

	/** Called when zombie detects target */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie|Events")
	void OnTargetDetected(AActor* DetectedTarget);
	virtual void OnTargetDetected_Implementation(AActor* DetectedTarget);

	/** Called when zombie loses target */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie|Events")
	void OnTargetLost(AActor* LostTarget);
	virtual void OnTargetLost_Implementation(AActor* LostTarget);

	/** Called when zombie kills target */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie|Events")
	void OnKilledTarget(AActor* VictimActor);
	virtual void OnKilledTarget_Implementation(AActor* VictimActor);

	// ========================================
	// NETWORK REPLICATION
	// ========================================

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========================================
	// DEBUG
	// ========================================

	/** Show debug information */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie|Debug")
	bool bShowDebugInfo = false;

	/** Draw debug information */
	UFUNCTION(BlueprintCallable, Category = "Zombie|Debug")
	void DrawDebugInfo();

protected:
	/** Current configuration */
	UPROPERTY(BlueprintReadOnly, Category = "Zombie|Config")
	FZombieConfigData CurrentConfig;

	/** Apply configuration to zombie */
	virtual void ApplyConfiguration(const FZombieConfigData& Config);

	/** Notify all abilities of damage */
	void NotifyAbilitiesOfDamage(float Damage, AActor* DamageSource);

	/** Notify all abilities of kill */
	void NotifyAbilitiesOfKill(AActor* VictimActor);

	/** Notify all abilities of target detection */
	void NotifyAbilitiesOfTargetDetected(AActor* DetectedActor);

	/** Notify all abilities of target loss */
	void NotifyAbilitiesOfTargetLost(AActor* LostActor);
};
