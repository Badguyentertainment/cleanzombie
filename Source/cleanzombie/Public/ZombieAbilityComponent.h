// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZombieAbilityComponent.generated.h"

/**
 * Base class for all zombie ability components
 * Provides a modular system for adding abilities to zombie variants
 *
 * Examples: ClimbingAbility, SpittingAbility, TunnelingAbility, etc.
 */
UCLASS(Abstract, Blueprintable, ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UZombieAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZombieAbilityComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// ABILITY LIFECYCLE
	// ========================================

	/**
	 * Called when ability is first initialized
	 * Override to set up ability-specific data
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie Ability")
	void InitializeAbility();
	virtual void InitializeAbility_Implementation();

	/**
	 * Called when ability should be activated
	 * @return true if successfully activated
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Ability")
	bool ActivateAbility();
	virtual bool ActivateAbility_Implementation();

	/**
	 * Called when ability should be deactivated
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Zombie Ability")
	void DeactivateAbility();
	virtual void DeactivateAbility_Implementation();

	/**
	 * Called every frame when ability is active
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie Ability")
	void UpdateAbility(float DeltaTime);
	virtual void UpdateAbility_Implementation(float DeltaTime);

	/**
	 * Check if this ability can be activated right now
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Zombie Ability")
	bool CanActivate() const;
	virtual bool CanActivate_Implementation() const;

	// ========================================
	// ABILITY STATE
	// ========================================

	/** Is this ability currently enabled? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability")
	bool bAbilityEnabled = true;

	/** Is this ability currently active? */
	UPROPERTY(BlueprintReadOnly, Category = "Zombie Ability")
	bool bIsActive = false;

	/** Priority of this ability (higher = more important) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability")
	int32 AbilityPriority = 0;

	/** Can this ability run simultaneously with other abilities? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability")
	bool bCanRunConcurrently = true;

	/** Tags that block this ability from activating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability")
	TArray<FName> BlockingTags;

	/** Tags this ability adds when active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability")
	TArray<FName> AbilityTags;

	// ========================================
	// ABILITY INFO
	// ========================================

	/** Display name of this ability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability|Info")
	FText AbilityName;

	/** Description of what this ability does */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability|Info")
	FText AbilityDescription;

	/** Icon for this ability (for UI/debug) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability|Info")
	class UTexture2D* AbilityIcon;

	// ========================================
	// ABILITY TARGETING
	// ========================================

	/** Current target actor for this ability */
	UPROPERTY(BlueprintReadWrite, Category = "Zombie Ability|Targeting")
	AActor* CurrentTarget;

	/**
	 * Set the target for this ability
	 */
	UFUNCTION(BlueprintCallable, Category = "Zombie Ability|Targeting")
	virtual void SetTarget(AActor* NewTarget);

	/**
	 * Get the current target
	 */
	UFUNCTION(BlueprintPure, Category = "Zombie Ability|Targeting")
	AActor* GetTarget() const { return CurrentTarget; }

	/**
	 * Check if ability has a valid target
	 */
	UFUNCTION(BlueprintPure, Category = "Zombie Ability|Targeting")
	bool HasValidTarget() const;

	// ========================================
	// ABILITY EVENTS
	// ========================================

	/**
	 * Called when zombie takes damage
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie Ability|Events")
	void OnZombieDamaged(float Damage, AActor* DamageSource);
	virtual void OnZombieDamaged_Implementation(float Damage, AActor* DamageSource);

	/**
	 * Called when zombie kills a target
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie Ability|Events")
	void OnZombieKilledTarget(AActor* VictimActor);
	virtual void OnZombieKilledTarget_Implementation(AActor* VictimActor);

	/**
	 * Called when zombie sees a new target
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie Ability|Events")
	void OnZombieDetectedTarget(AActor* DetectedActor);
	virtual void OnZombieDetectedTarget_Implementation(AActor* DetectedActor);

	/**
	 * Called when zombie loses sight of target
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie Ability|Events")
	void OnZombieLostTarget(AActor* LostActor);
	virtual void OnZombieLostTarget_Implementation(AActor* LostActor);

	// ========================================
	// DEBUG
	// ========================================

	/** Enable debug visualization for this ability */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie Ability|Debug")
	bool bShowDebug = false;

	/**
	 * Draw debug information
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Zombie Ability|Debug")
	void DrawDebugInfo();
	virtual void DrawDebugInfo_Implementation();

	/**
	 * Get debug string for this ability
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Zombie Ability|Debug")
	FString GetDebugString() const;
	virtual FString GetDebugString_Implementation() const;

protected:
	// ========================================
	// CACHED REFERENCES
	// ========================================

	/** Cached owner character */
	UPROPERTY(BlueprintReadOnly, Category = "Zombie Ability")
	class ACharacter* OwnerZombie;

	/** Cached zombie base reference */
	UPROPERTY(BlueprintReadOnly, Category = "Zombie Ability")
	class AZombieBase* ZombieBase;

	/** Time this ability has been active */
	UPROPERTY(BlueprintReadOnly, Category = "Zombie Ability")
	float TimeActive = 0.0f;

	/** Active ability tags on this zombie */
	TArray<FName> ActiveTags;

	/**
	 * Helper: Add ability tags when activated
	 */
	void AddAbilityTags();

	/**
	 * Helper: Remove ability tags when deactivated
	 */
	void RemoveAbilityTags();

	/**
	 * Helper: Check if any blocking tags are active
	 */
	bool HasBlockingTags() const;
};
