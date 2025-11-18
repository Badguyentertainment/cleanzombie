// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "TankAbility.generated.h"

UENUM(BlueprintType)
enum class EChargeState : uint8
{
	Idle,
	Preparing,
	Charging,
	Recovering
};

/**
 * Tank zombie ability - heavy charger that smashes through obstacles
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UTankAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	UTankAbility();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Execute charge */
	UFUNCTION(BlueprintCallable, Category = "Tank")
	void ExecuteCharge();

	/** Can charge? */
	UFUNCTION(BlueprintPure, Category = "Tank")
	bool CanCharge() const;

	/** Is charging? */
	UFUNCTION(BlueprintPure, Category = "Tank")
	bool IsCharging() const { return CurrentChargeState == EChargeState::Charging; }

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank|Config")
	float ChargeSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank|Config")
	float ChargeDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank|Config")
	float ChargeDamage = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank|Config")
	float KnockbackForce = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank|Config")
	float ChargeCooldown = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank|Config")
	float PreparationTime = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank|Config")
	bool bCanBreakObstacles = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tank|Config")
	float ObstacleDamage = 500.0f;

	// Events
	UFUNCTION(BlueprintNativeEvent, Category = "Tank|Events")
	void OnChargeStarted();

	UFUNCTION(BlueprintNativeEvent, Category = "Tank|Events")
	void OnChargeImpact(AActor* HitActor, float Damage);

	UFUNCTION(BlueprintNativeEvent, Category = "Tank|Events")
	void OnObstacleDestroyed(AActor* Obstacle);

	UFUNCTION(BlueprintNativeEvent, Category = "Tank|Events")
	void OnChargeEnded();

protected:
	UPROPERTY(BlueprintReadOnly)
	EChargeState CurrentChargeState = EChargeState::Idle;

	UPROPERTY(BlueprintReadOnly)
	AActor* ChargeTarget = nullptr;

	float ChargeElapsedTime = 0.0f;
	float PreparationElapsedTime = 0.0f;
	float LastChargeTime = -999.0f;
	FVector ChargeDirection;

	void UpdateCharge(float DeltaTime);
	void CheckChargeCollisions();
	void ApplyChargeDamage(AActor* Target);
	void BreakObstacle(AActor* Obstacle);
	AActor* FindChargeTarget();

	virtual void OnChargeStarted_Implementation();
	virtual void OnChargeImpact_Implementation(AActor* HitActor, float Damage);
	virtual void OnObstacleDestroyed_Implementation(AActor* Obstacle);
	virtual void OnChargeEnded_Implementation();
};
