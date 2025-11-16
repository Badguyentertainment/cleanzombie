// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBase.h"
#include "WWGZombieIntegration.generated.h"

/**
 * Integration class for WWG Mutating Zombie Default asset
 * Bridges our modular ability system with WWG's transformation and dismemberment
 *
 * SETUP:
 * 1. Import WWG_ZombieDefault from Marketplace
 * 2. Create Blueprint from WWG's zombie character
 * 3. Add this component OR reparent to UWWGZombieBase
 * 4. Configure zombie abilities as normal
 */

/**
 * Zombification state for progressive transformation
 */
UENUM(BlueprintType)
enum class EZombificationState : uint8
{
	Human,				// 0% - Fully human
	EarlyInfection,		// 25% - Initial symptoms
	MidInfection,		// 50% - Visible mutation
	LateInfection,		// 75% - Advanced decay
	FullZombie			// 100% - Complete transformation
};

/**
 * Dismembered limb tracking
 */
USTRUCT(BlueprintType)
struct FDismemberedLimb
{
	GENERATED_BODY()

	/** Which limb was dismembered */
	UPROPERTY(BlueprintReadWrite)
	FName LimbName;

	/** Time when dismembered */
	UPROPERTY(BlueprintReadWrite)
	float DismemberTime = 0.0f;

	/** Still functional after dismemberment? */
	UPROPERTY(BlueprintReadWrite)
	bool bStillFunctional = false;

	/** Damage that caused dismemberment */
	UPROPERTY(BlueprintReadWrite)
	float DismemberDamage = 0.0f;

	FDismemberedLimb()
		: LimbName(NAME_None)
		, DismemberTime(0.0f)
		, bStillFunctional(false)
		, DismemberDamage(0.0f)
	{
	}
};

/**
 * Component that integrates WWG Zombie features with our ability system
 * Add to WWG zombie Blueprint to enable our modular abilities
 */
UCLASS(ClassGroup = (ZombieIntegration), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UWWGZombieIntegration : public UActorComponent
{
	GENERATED_BODY()

public:
	UWWGZombieIntegration();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// ZOMBIFICATION SYSTEM
	// ========================================

	/** Current zombification level (0.0 = human, 1.0 = full zombie) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Zombification", Replicated, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ZombificationLevel = 1.0f;

	/** Rate of zombification per second (for infection mechanics) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Zombification")
	float ZombificationRate = 0.1f;

	/** Is zombification progressing? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Zombification")
	bool bIsInfected = false;

	/** Can zombification be reversed? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Zombification")
	bool bCanCureInfection = true;

	/** Set zombification level */
	UFUNCTION(BlueprintCallable, Category = "WWG Integration|Zombification")
	void SetZombificationLevel(float NewLevel);

	/** Infect this character (start transformation) */
	UFUNCTION(BlueprintCallable, Category = "WWG Integration|Zombification")
	void InfectCharacter(float InitialLevel = 0.0f);

	/** Cure infection (reverse transformation) */
	UFUNCTION(BlueprintCallable, Category = "WWG Integration|Zombification")
	void CureInfection();

	/** Get current zombification state */
	UFUNCTION(BlueprintPure, Category = "WWG Integration|Zombification")
	EZombificationState GetZombificationState() const;

	/** Is fully transformed? */
	UFUNCTION(BlueprintPure, Category = "WWG Integration|Zombification")
	bool IsFullyZombified() const { return ZombificationLevel >= 0.99f; }

	// ========================================
	// DISMEMBERMENT SYSTEM
	// ========================================

	/** Track dismembered limbs */
	UPROPERTY(BlueprintReadOnly, Category = "WWG Integration|Dismemberment", Replicated)
	TArray<FDismemberedLimb> DismemberedLimbs;

	/** Chance to dismember on heavy damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Dismemberment")
	float DismemberChance = 0.3f;

	/** Minimum damage to trigger dismemberment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Dismemberment")
	float MinDismemberDamage = 50.0f;

	/** Can zombie function with missing limbs? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Dismemberment")
	bool bCanSurviveDismemberment = true;

	/** Dismember a limb */
	UFUNCTION(BlueprintCallable, Category = "WWG Integration|Dismemberment")
	bool DismemberLimb(FName LimbName, float Damage, bool bInstantKill = false);

	/** Is limb dismembered? */
	UFUNCTION(BlueprintPure, Category = "WWG Integration|Dismemberment")
	bool IsLimbDismembered(FName LimbName) const;

	/** Get dismembered limb count */
	UFUNCTION(BlueprintPure, Category = "WWG Integration|Dismemberment")
	int32 GetDismemberedLimbCount() const { return DismemberedLimbs.Num(); }

	/** Can zombie still function? */
	UFUNCTION(BlueprintPure, Category = "WWG Integration|Dismemberment")
	bool CanStillFunction() const;

	// ========================================
	// DIRT/DECAY SYSTEM
	// ========================================

	/** Current dirt level (0.0 = clean, 1.0 = very dirty) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Appearance", Replicated, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DirtLevel = 0.5f;

	/** Automatically increase dirt over time? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Appearance")
	bool bAccumulateDirt = true;

	/** Dirt accumulation rate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WWG Integration|Appearance")
	float DirtAccumulationRate = 0.01f;

	/** Set dirt level */
	UFUNCTION(BlueprintCallable, Category = "WWG Integration|Appearance")
	void SetDirtLevel(float NewLevel);

	// ========================================
	// ABILITY SYSTEM INTEGRATION
	// ========================================

	/** Reference to ZombieBase (if using inheritance model) */
	UPROPERTY(BlueprintReadOnly, Category = "WWG Integration|Abilities")
	AZombieBase* ZombieBase = nullptr;

	/** Get all abilities on this zombie */
	UFUNCTION(BlueprintCallable, Category = "WWG Integration|Abilities")
	TArray<UZombieAbilityComponent*> GetAllAbilities() const;

	/** Modify ability based on zombification level */
	UFUNCTION(BlueprintCallable, Category = "WWG Integration|Abilities")
	void ApplyZombificationToAbilities();

	/** Disable abilities based on dismemberment */
	UFUNCTION(BlueprintCallable, Category = "WWG Integration|Abilities")
	void ApplyDismembermentToAbilities();

	// ========================================
	// EVENTS
	// ========================================

	/** Called when zombification level changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "WWG Integration|Events")
	void OnZombificationChanged(float OldLevel, float NewLevel);

	/** Called when fully transformed */
	UFUNCTION(BlueprintImplementableEvent, Category = "WWG Integration|Events")
	void OnFullyZombified();

	/** Called when limb dismembered */
	UFUNCTION(BlueprintImplementableEvent, Category = "WWG Integration|Events")
	void OnLimbDismembered(FName LimbName, float Damage);

	/** Called when infection cured */
	UFUNCTION(BlueprintImplementableEvent, Category = "WWG Integration|Events")
	void OnInfectionCured();

protected:
	/** Update zombification */
	void UpdateZombification(float DeltaTime);

	/** Update appearance based on state */
	void UpdateAppearance();

	/** Apply movement penalties from dismemberment */
	void ApplyDismembermentEffects();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};

/**
 * Alternative: Base class that extends ZombieBase with WWG integration
 * Use this if you want to reparent WWG zombie Blueprint to C++ class
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API AWWGZombieBase : public AZombieBase
{
	GENERATED_BODY()

public:
	AWWGZombieBase();

	// ========================================
	// WWG INTEGRATION (Built-in)
	// ========================================

	/** WWG integration component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWWGZombieIntegration* WWGIntegration;

	/** Quick access to zombification level */
	UFUNCTION(BlueprintCallable, Category = "WWG Zombie")
	float GetZombificationLevel() const;

	/** Quick access to set zombification */
	UFUNCTION(BlueprintCallable, Category = "WWG Zombie")
	void SetZombificationLevel(float NewLevel);

	/** Quick access to dismember */
	UFUNCTION(BlueprintCallable, Category = "WWG Zombie")
	bool DismemberLimb(FName LimbName, float Damage);

protected:
	virtual void BeginPlay() override;
};
