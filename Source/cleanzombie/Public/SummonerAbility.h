// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "SummonerAbility.generated.h"

/**
 * Summoner zombie ability - spawns minion zombies
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API USummonerAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	USummonerAbility();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Summon minions */
	UFUNCTION(BlueprintCallable, Category = "Summoner")
	void SummonMinions();

	/** Can summon? */
	UFUNCTION(BlueprintPure, Category = "Summoner")
	bool CanSummon() const;

	/** Get active minion count */
	UFUNCTION(BlueprintPure, Category = "Summoner")
	int32 GetActiveMinionCount() const { return ActiveMinions.Num(); }

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config")
	TSubclassOf<class AZombieBase> MinionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config")
	int32 MinionsPerSummon = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config")
	int32 MaxActiveMinions = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config")
	float SummonRadius = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config")
	float SummonCooldown = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config")
	float SummonChannelTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config")
	bool bMinionsSurviveOnDeath = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config|Visuals")
	UParticleSystem* SummonParticle = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Summoner|Config|Visuals")
	USoundBase* SummonSound = nullptr;

	// Events
	UFUNCTION(BlueprintNativeEvent, Category = "Summoner|Events")
	void OnSummonStarted();

	UFUNCTION(BlueprintNativeEvent, Category = "Summoner|Events")
	void OnMinionSpawned(AZombieBase* Minion);

protected:
	UPROPERTY(BlueprintReadOnly)
	TArray<AZombieBase*> ActiveMinions;

	float LastSummonTime = -999.0f;
	bool bIsSummoning = false;
	float SummonElapsedTime = 0.0f;

	void ExecuteSummon();
	void SpawnMinion(FVector Location);
	void CleanupDeadMinions();

	virtual void OnSummonStarted_Implementation();
	virtual void OnMinionSpawned_Implementation(AZombieBase* Minion);
};
