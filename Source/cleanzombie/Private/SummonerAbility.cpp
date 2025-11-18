// Copyright Epic Games, Inc. All Rights Reserved.

#include "SummonerAbility.h"
#include "ZombieBase.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

USummonerAbility::USummonerAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	AbilityName = TEXT("Summoner");
	AbilityDescription = TEXT("Spawns minion zombies");
	AbilityTags.Add(TEXT("Summoner"));
	AbilityTags.Add(TEXT("Spawn"));
}

void USummonerAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsActive || !OwnerZombie) return;

	CleanupDeadMinions();

	if (bIsSummoning)
	{
		SummonElapsedTime += DeltaTime;
		if (SummonElapsedTime >= SummonChannelTime)
		{
			ExecuteSummon();
			bIsSummoning = false;
		}
	}
	else if (CanSummon())
	{
		SummonMinions();
	}
}

void USummonerAbility::SummonMinions()
{
	if (!CanSummon()) return;

	bIsSummoning = true;
	SummonElapsedTime = 0.0f;
	LastSummonTime = GetWorld()->GetTimeSeconds();

	OnSummonStarted();
}

bool USummonerAbility::CanSummon() const
{
	return bIsActive && OwnerZombie && !bIsSummoning && MinionClass &&
		ActiveMinions.Num() < MaxActiveMinions &&
		(GetWorld()->GetTimeSeconds() - LastSummonTime >= SummonCooldown);
}

void USummonerAbility::ExecuteSummon()
{
	if (!OwnerZombie || !MinionClass) return;

	for (int32 i = 0; i < MinionsPerSummon; i++)
	{
		if (ActiveMinions.Num() >= MaxActiveMinions) break;

		// Find random spawn location around summoner
		FVector BaseLocation = OwnerZombie->GetActorLocation();
		float Angle = (360.0f / MinionsPerSummon) * i;
		FVector Offset = FVector(
			FMath::Cos(FMath::DegreesToRadians(Angle)) * SummonRadius,
			FMath::Sin(FMath::DegreesToRadians(Angle)) * SummonRadius,
			0.0f
		);

		FVector SpawnLocation = BaseLocation + Offset;

		// Find navmesh location
		UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(GetWorld());
		if (NavSys)
		{
			FNavLocation NavLoc;
			if (NavSys->ProjectPointToNavigation(SpawnLocation, NavLoc, FVector(500, 500, 500)))
			{
				SpawnLocation = NavLoc.Location;
			}
		}

		SpawnMinion(SpawnLocation);
	}

	// Play effects
	if (SummonSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SummonSound, OwnerZombie->GetActorLocation());
	}

	if (SummonParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SummonParticle, OwnerZombie->GetActorLocation());
	}
}

void USummonerAbility::SpawnMinion(FVector Location)
{
	if (!MinionClass) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AZombieBase* Minion = GetWorld()->SpawnActor<AZombieBase>(
		MinionClass,
		Location,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (Minion)
	{
		ActiveMinions.Add(Minion);
		OnMinionSpawned(Minion);

		// Spawn effect at minion location
		if (SummonParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SummonParticle, Location);
		}
	}
}

void USummonerAbility::CleanupDeadMinions()
{
	for (int32 i = ActiveMinions.Num() - 1; i >= 0; i--)
	{
		if (!ActiveMinions[i] || !IsValid(ActiveMinions[i]) || ActiveMinions[i]->IsPendingKillPending())
		{
			ActiveMinions.RemoveAt(i);
		}
	}
}

void USummonerAbility::OnSummonStarted_Implementation() {}
void USummonerAbility::OnMinionSpawned_Implementation(AZombieBase* Minion) {}
