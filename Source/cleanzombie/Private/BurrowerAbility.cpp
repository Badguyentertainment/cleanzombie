// Copyright Epic Games, Inc. All Rights Reserved.

#include "BurrowerAbility.h"
#include "ZombieBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "StatusEffectComponent.h"

UBurrowerAbility::UBurrowerAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	AbilityName = TEXT("Burrower");
	AbilityDescription = TEXT("Digs underground for surprise attacks");
	AbilityTags.Add(TEXT("Burrower"));
	AbilityTags.Add(TEXT("Stealth"));
}

void UBurrowerAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsActive || !OwnerZombie) return;

	switch (CurrentBurrowState)
	{
	case EBurrowState::Idle:
		if (CanBurrow())
		{
			Burrow();
		}
		break;

	case EBurrowState::Underground:
		UpdateUndergroundMovement(DeltaTime);
		break;

	default:
		break;
	}
}

void UBurrowerAbility::Burrow()
{
	if (!CanBurrow()) return;

	CurrentBurrowState = EBurrowState::Burrowing;
	BurrowStartLocation = OwnerZombie->GetActorLocation();
	UndergroundElapsedTime = 0.0f;
	LastBurrowTime = GetWorld()->GetTimeSeconds();

	// Make invisible
	if (bInvisibleUnderground)
	{
		OwnerZombie->SetActorHiddenInGame(true);
		OwnerZombie->SetActorEnableCollision(false);
	}

	// Apply invisible status
	UStatusEffectComponent* Effects = OwnerZombie->FindComponentByClass<UStatusEffectComponent>();
	if (Effects)
	{
		Effects->ApplyStatusEffect(EStatusEffectType::Invisible, 1.0f, MaxUndergroundTime, OwnerZombie);
	}

	CurrentBurrowState = EBurrowState::Underground;
	OnBurrowed();
}

void UBurrowerAbility::Emerge(FVector Location)
{
	if (CurrentBurrowState != EBurrowState::Underground) return;

	TargetEmergenceLocation = Location;
	OwnerZombie->SetActorLocation(Location);

	// Make visible
	OwnerZombie->SetActorHiddenInGame(false);
	OwnerZombie->SetActorEnableCollision(true);

	// Deal emerge damage
	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, Location, Location, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(EmergeRadius));

	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == OwnerZombie) continue;

		UGameplayStatics::ApplyDamage(Target, EmergeDamage, OwnerZombie->GetInstigatorController(),
			OwnerZombie, UDamageType::StaticClass());

		// Knockup
		ACharacter* TargetChar = Cast<ACharacter>(Target);
		if (TargetChar)
		{
			TargetChar->LaunchCharacter(FVector(0, 0, 1000.0f), true, true);
		}
	}

	CurrentBurrowState = EBurrowState::Idle;
	OnEmerged(Location);
}

bool UBurrowerAbility::CanBurrow() const
{
	return bIsActive && OwnerZombie && CurrentBurrowState == EBurrowState::Idle &&
		(GetWorld()->GetTimeSeconds() - LastBurrowTime >= BurrowCooldown);
}

void UBurrowerAbility::UpdateUndergroundMovement(float DeltaTime)
{
	UndergroundElapsedTime += DeltaTime;

	// Auto-emerge if max time reached
	if (UndergroundElapsedTime >= MaxUndergroundTime)
	{
		AActor* Target = FindBestEmergeTarget();
		FVector EmergeLocation = Target ? Target->GetActorLocation() : BurrowStartLocation;
		Emerge(EmergeLocation);
		return;
	}

	// Move toward target underground
	AActor* Target = FindBestEmergeTarget();
	if (Target)
	{
		FVector Direction = (Target->GetActorLocation() - OwnerZombie->GetActorLocation()).GetSafeNormal();
		FVector NewLocation = OwnerZombie->GetActorLocation() + (Direction * BurrowSpeed * DeltaTime);
		OwnerZombie->SetActorLocation(NewLocation);

		// Emerge when close
		if (FVector::Dist(OwnerZombie->GetActorLocation(), Target->GetActorLocation()) < 200.0f)
		{
			Emerge(Target->GetActorLocation());
		}
	}
}

AActor* UBurrowerAbility::FindBestEmergeTarget()
{
	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, OwnerZombie->GetActorLocation(),
		OwnerZombie->GetActorLocation(), FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(2000.0f));

	AActor* Best = nullptr;
	float BestDist = FLT_MAX;

	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == OwnerZombie) continue;

		float Dist = FVector::Dist(OwnerZombie->GetActorLocation(), Target->GetActorLocation());
		if (Dist < BestDist)
		{
			BestDist = Dist;
			Best = Target;
		}
	}

	return Best;
}

void UBurrowerAbility::OnBurrowed_Implementation() {}
void UBurrowerAbility::OnEmerged_Implementation(FVector Location) {}
