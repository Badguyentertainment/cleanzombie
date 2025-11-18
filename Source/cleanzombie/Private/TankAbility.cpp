// Copyright Epic Games, Inc. All Rights Reserved.

#include "TankAbility.h"
#include "ZombieBase.h"
#include "BarricadeActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "StatusEffectComponent.h"

UTankAbility::UTankAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	AbilityName = TEXT("Tank");
	AbilityDescription = TEXT("Heavy charge that smashes obstacles");
	AbilityTags.Add(TEXT("Tank"));
	AbilityTags.Add(TEXT("Charge"));
}

void UTankAbility::BeginPlay()
{
	Super::BeginPlay();
}

void UTankAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsActive || !OwnerZombie) return;

	switch (CurrentChargeState)
	{
	case EChargeState::Idle:
		if (CanCharge())
		{
			ChargeTarget = FindChargeTarget();
			if (ChargeTarget)
			{
				CurrentChargeState = EChargeState::Preparing;
				PreparationElapsedTime = 0.0f;
			}
		}
		break;

	case EChargeState::Preparing:
		PreparationElapsedTime += DeltaTime;
		if (PreparationElapsedTime >= PreparationTime)
		{
			ExecuteCharge();
		}
		break;

	case EChargeState::Charging:
		UpdateCharge(DeltaTime);
		break;

	case EChargeState::Recovering:
		if (GetWorld()->GetTimeSeconds() - LastChargeTime >= ChargeCooldown)
		{
			CurrentChargeState = EChargeState::Idle;
		}
		break;
	}
}

void UTankAbility::ExecuteCharge()
{
	if (!CanCharge() || !ChargeTarget) return;

	ChargeDirection = (ChargeTarget->GetActorLocation() - OwnerZombie->GetActorLocation()).GetSafeNormal();
	ChargeElapsedTime = 0.0f;
	CurrentChargeState = EChargeState::Charging;
	LastChargeTime = GetWorld()->GetTimeSeconds();

	OnChargeStarted();
}

bool UTankAbility::CanCharge() const
{
	return bIsActive && OwnerZombie &&
		CurrentChargeState == EChargeState::Idle &&
		(GetWorld()->GetTimeSeconds() - LastChargeTime >= ChargeCooldown);
}

void UTankAbility::UpdateCharge(float DeltaTime)
{
	ChargeElapsedTime += DeltaTime;

	if (ChargeElapsedTime >= ChargeDuration)
	{
		CurrentChargeState = EChargeState::Recovering;
		OnChargeEnded();
		return;
	}

	// Move forward at charge speed
	FVector NewLocation = OwnerZombie->GetActorLocation() + (ChargeDirection * ChargeSpeed * DeltaTime);
	OwnerZombie->SetActorLocation(NewLocation, true);

	CheckChargeCollisions();
}

void UTankAbility::CheckChargeCollisions()
{
	TArray<FHitResult> Hits;
	FVector Start = OwnerZombie->GetActorLocation();
	FVector End = Start + ChargeDirection * 100.0f;

	GetWorld()->SweepMultiByChannel(Hits, Start, End, FQuat::Identity,
		ECC_Pawn, FCollisionShape::MakeSphere(100.0f));

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == OwnerZombie) continue;

		// Check if obstacle
		if (bCanBreakObstacles && Cast<ABarricadeActor>(HitActor))
		{
			BreakObstacle(HitActor);
		}
		else
		{
			ApplyChargeDamage(HitActor);
		}
	}
}

void UTankAbility::ApplyChargeDamage(AActor* Target)
{
	UGameplayStatics::ApplyDamage(Target, ChargeDamage, OwnerZombie->GetInstigatorController(),
		OwnerZombie, UDamageType::StaticClass());

	// Apply knockback
	ACharacter* TargetChar = Cast<ACharacter>(Target);
	if (TargetChar)
	{
		TargetChar->LaunchCharacter(ChargeDirection * KnockbackForce, true, true);
	}

	// Apply stun
	UStatusEffectComponent* Effects = Target->FindComponentByClass<UStatusEffectComponent>();
	if (Effects)
	{
		Effects->ApplyStatusEffect(EStatusEffectType::Stun, 1.0f, 1.0f, OwnerZombie);
	}

	OnChargeImpact(Target, ChargeDamage);
}

void UTankAbility::BreakObstacle(AActor* Obstacle)
{
	UGameplayStatics::ApplyDamage(Obstacle, ObstacleDamage, OwnerZombie->GetInstigatorController(),
		OwnerZombie, UDamageType::StaticClass());

	OnObstacleDestroyed(Obstacle);
}

AActor* UTankAbility::FindChargeTarget()
{
	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, OwnerZombie->GetActorLocation(),
		OwnerZombie->GetActorLocation(), FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(1500.0f));

	AActor* Best = nullptr;
	float BestScore = -1.0f;

	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == OwnerZombie) continue;

		FVector ToTarget = (Target->GetActorLocation() - OwnerZombie->GetActorLocation()).GetSafeNormal();
		float Dot = FVector::DotProduct(OwnerZombie->GetActorForwardVector(), ToTarget);

		if (Dot > BestScore)
		{
			BestScore = Dot;
			Best = Target;
		}
	}

	return Best;
}

void UTankAbility::OnChargeStarted_Implementation() {}
void UTankAbility::OnChargeImpact_Implementation(AActor* HitActor, float Damage) {}
void UTankAbility::OnObstacleDestroyed_Implementation(AActor* Obstacle) {}
void UTankAbility::OnChargeEnded_Implementation() {}
