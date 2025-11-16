// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZombieAbilityComponent.h"
#include "ZombieBase.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

UZombieAbilityComponent::UZombieAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bAutoActivate = true;
}

void UZombieAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache owner references
	OwnerZombie = Cast<ACharacter>(GetOwner());
	ZombieBase = Cast<AZombieBase>(GetOwner());

	// Initialize ability
	InitializeAbility();
}

void UZombieAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsActive)
	{
		TimeActive += DeltaTime;
		UpdateAbility(DeltaTime);

		if (bShowDebug)
		{
			DrawDebugInfo();
		}
	}
}

// ========================================
// ABILITY LIFECYCLE
// ========================================

void UZombieAbilityComponent::InitializeAbility_Implementation()
{
	// Override in child classes
}

bool UZombieAbilityComponent::ActivateAbility_Implementation()
{
	if (!bAbilityEnabled || bIsActive)
	{
		return false;
	}

	if (!CanActivate())
	{
		return false;
	}

	if (HasBlockingTags())
	{
		return false;
	}

	bIsActive = true;
	TimeActive = 0.0f;
	AddAbilityTags();

	return true;
}

void UZombieAbilityComponent::DeactivateAbility_Implementation()
{
	if (!bIsActive)
	{
		return;
	}

	bIsActive = false;
	TimeActive = 0.0f;
	RemoveAbilityTags();
}

void UZombieAbilityComponent::UpdateAbility_Implementation(float DeltaTime)
{
	// Override in child classes
}

bool UZombieAbilityComponent::CanActivate_Implementation() const
{
	return bAbilityEnabled && OwnerZombie != nullptr;
}

// ========================================
// ABILITY TARGETING
// ========================================

void UZombieAbilityComponent::SetTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
}

bool UZombieAbilityComponent::HasValidTarget() const
{
	return CurrentTarget != nullptr && IsValid(CurrentTarget);
}

// ========================================
// ABILITY EVENTS
// ========================================

void UZombieAbilityComponent::OnZombieDamaged_Implementation(float Damage, AActor* DamageSource)
{
	// Override in child classes
}

void UZombieAbilityComponent::OnZombieKilledTarget_Implementation(AActor* VictimActor)
{
	// Override in child classes
}

void UZombieAbilityComponent::OnZombieDetectedTarget_Implementation(AActor* DetectedActor)
{
	// Override in child classes
}

void UZombieAbilityComponent::OnZombieLostTarget_Implementation(AActor* LostActor)
{
	// Override in child classes
}

// ========================================
// DEBUG
// ========================================

void UZombieAbilityComponent::DrawDebugInfo_Implementation()
{
	if (!OwnerZombie)
	{
		return;
	}

	FVector Location = OwnerZombie->GetActorLocation() + FVector(0, 0, 100);
	FString DebugText = GetDebugString();

	DrawDebugString(
		GetWorld(),
		Location,
		DebugText,
		nullptr,
		bIsActive ? FColor::Green : FColor::Yellow,
		0.0f,
		true
	);
}

FString UZombieAbilityComponent::GetDebugString_Implementation() const
{
	return FString::Printf(
		TEXT("%s: %s (Priority: %d)"),
		*AbilityName.ToString(),
		bIsActive ? TEXT("ACTIVE") : TEXT("Inactive"),
		AbilityPriority
	);
}

// ========================================
// HELPER FUNCTIONS
// ========================================

void UZombieAbilityComponent::AddAbilityTags()
{
	for (const FName& Tag : AbilityTags)
	{
		if (!ActiveTags.Contains(Tag))
		{
			ActiveTags.Add(Tag);
		}
	}
}

void UZombieAbilityComponent::RemoveAbilityTags()
{
	for (const FName& Tag : AbilityTags)
	{
		ActiveTags.Remove(Tag);
	}
}

bool UZombieAbilityComponent::HasBlockingTags() const
{
	for (const FName& BlockingTag : BlockingTags)
	{
		if (ActiveTags.Contains(BlockingTag))
		{
			return true;
		}
	}
	return false;
}
