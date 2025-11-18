// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScreamerAbility.h"
#include "ZombieBase.h"
#include "Kismet/GameplayStatics.h"
#include "StatusEffectComponent.h"

UScreamerAbility::UScreamerAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	AbilityName = TEXT("Screamer");
	AbilityDescription = TEXT("Sonic scream stuns and disorients");
	AbilityTags.Add(TEXT("Screamer"));
	AbilityTags.Add(TEXT("AoE"));
}

void UScreamerAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsActive || !OwnerZombie) return;

	if (bIsScreaming)
	{
		WindupElapsedTime += DeltaTime;
		if (WindupElapsedTime >= WindupTime)
		{
			ApplyScreamEffects();
			bIsScreaming = false;
		}
	}
	else if (CanScream())
	{
		ExecuteScream();
	}
}

void UScreamerAbility::ExecuteScream()
{
	if (!CanScream()) return;

	bIsScreaming = true;
	WindupElapsedTime = 0.0f;
	LastScreamTime = GetWorld()->GetTimeSeconds();

	OnScreamStarted();
}

bool UScreamerAbility::CanScream() const
{
	return bIsActive && OwnerZombie && !bIsScreaming &&
		(GetWorld()->GetTimeSeconds() - LastScreamTime >= ScreamCooldown);
}

void UScreamerAbility::ApplyScreamEffects()
{
	if (!OwnerZombie) return;

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByChannel(Hits, OwnerZombie->GetActorLocation(),
		OwnerZombie->GetActorLocation(), FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(ScreamRadius));

	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == OwnerZombie) continue;

		// Apply damage
		UGameplayStatics::ApplyDamage(Target, ScreamDamage, OwnerZombie->GetInstigatorController(),
			OwnerZombie, UDamageType::StaticClass());

		// Apply status effects
		UStatusEffectComponent* Effects = Target->FindComponentByClass<UStatusEffectComponent>();
		if (Effects)
		{
			Effects->ApplyStatusEffect(EStatusEffectType::Stun, 1.0f, StunDuration, OwnerZombie);

			if (bCausesBlinding)
			{
				Effects->ApplyStatusEffect(EStatusEffectType::Blinding, 0.8f, StunDuration * 2.0f, OwnerZombie);
			}

			if (bCausesConfusion)
			{
				Effects->ApplyStatusEffect(EStatusEffectType::Confused, 1.0f, StunDuration * 1.5f, OwnerZombie);
			}
		}

		OnActorAffected(Target);
	}

	// Play effects
	if (ScreamSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ScreamSound, OwnerZombie->GetActorLocation());
	}

	if (ScreamParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ScreamParticle,
			OwnerZombie->GetActorLocation());
	}
}

void UScreamerAbility::OnScreamStarted_Implementation() {}
void UScreamerAbility::OnActorAffected_Implementation(AActor* Target) {}
