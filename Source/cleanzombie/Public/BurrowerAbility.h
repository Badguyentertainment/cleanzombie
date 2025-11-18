// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "BurrowerAbility.generated.h"

UENUM(BlueprintType)
enum class EBurrowState : uint8
{
	Idle,
	Burrowing,
	Underground,
	Emerging
};

/**
 * Burrower zombie ability - digs underground for surprise attacks
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UBurrowerAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	UBurrowerAbility();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Burrow underground */
	UFUNCTION(BlueprintCallable, Category = "Burrower")
	void Burrow();

	/** Emerge at location */
	UFUNCTION(BlueprintCallable, Category = "Burrower")
	void Emerge(FVector Location);

	/** Can burrow? */
	UFUNCTION(BlueprintPure, Category = "Burrower")
	bool CanBurrow() const;

	/** Is underground? */
	UFUNCTION(BlueprintPure, Category = "Burrower")
	bool IsUnderground() const { return CurrentBurrowState == EBurrowState::Underground; }

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burrower|Config")
	float BurrowSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burrower|Config")
	float MaxUndergroundTime = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burrower|Config")
	float EmergeDamage = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burrower|Config")
	float EmergeRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burrower|Config")
	float BurrowCooldown = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Burrower|Config")
	bool bInvisibleUnderground = true;

	// Events
	UFUNCTION(BlueprintNativeEvent, Category = "Burrower|Events")
	void OnBurrowed();

	UFUNCTION(BlueprintNativeEvent, Category = "Burrower|Events")
	void OnEmerged(FVector Location);

protected:
	UPROPERTY(BlueprintReadOnly)
	EBurrowState CurrentBurrowState = EBurrowState::Idle;

	float UndergroundElapsedTime = 0.0f;
	float LastBurrowTime = -999.0f;
	FVector BurrowStartLocation;
	FVector TargetEmergenceLocation;

	void UpdateUndergroundMovement(float DeltaTime);
	AActor* FindBestEmergeTarget();

	virtual void OnBurrowed_Implementation();
	virtual void OnEmerged_Implementation(FVector Location);
};
