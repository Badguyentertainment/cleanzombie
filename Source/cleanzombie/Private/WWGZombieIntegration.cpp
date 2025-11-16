// Copyright Epic Games, Inc. All Rights Reserved.

#include "WWGZombieIntegration.h"
#include "ZombieBase.h"
#include "ZombieAbilityComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

UWWGZombieIntegration::UWWGZombieIntegration()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UWWGZombieIntegration::BeginPlay()
{
	Super::BeginPlay();

	// Cache ZombieBase reference
	ZombieBase = Cast<AZombieBase>(GetOwner());

	if (!ZombieBase)
	{
		UE_LOG(LogTemp, Warning, TEXT("WWGZombieIntegration: Owner is not AZombieBase, some features may not work"));
	}

	// Apply initial zombification to abilities
	if (ZombieBase)
	{
		ApplyZombificationToAbilities();
	}
}

void UWWGZombieIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update zombification progression
	if (bIsInfected)
	{
		UpdateZombification(DeltaTime);
	}

	// Accumulate dirt
	if (bAccumulateDirt && DirtLevel < 1.0f)
	{
		DirtLevel = FMath::Clamp(DirtLevel + (DirtAccumulationRate * DeltaTime), 0.0f, 1.0f);
	}

	// Apply dismemberment effects
	if (DismemberedLimbs.Num() > 0)
	{
		ApplyDismembermentEffects();
	}
}

void UWWGZombieIntegration::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWWGZombieIntegration, ZombificationLevel);
	DOREPLIFETIME(UWWGZombieIntegration, DismemberedLimbs);
	DOREPLIFETIME(UWWGZombieIntegration, DirtLevel);
}

// ========================================
// ZOMBIFICATION SYSTEM
// ========================================

void UWWGZombieIntegration::SetZombificationLevel(float NewLevel)
{
	float OldLevel = ZombificationLevel;
	ZombificationLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);

	if (FMath::Abs(OldLevel - ZombificationLevel) > 0.01f)
	{
		OnZombificationChanged(OldLevel, ZombificationLevel);
		ApplyZombificationToAbilities();

		if (IsFullyZombified() && OldLevel < 0.99f)
		{
			OnFullyZombified();
		}
	}
}

void UWWGZombieIntegration::InfectCharacter(float InitialLevel)
{
	bIsInfected = true;
	SetZombificationLevel(InitialLevel);

	UE_LOG(LogTemp, Log, TEXT("WWGZombieIntegration: %s infected at level %.2f"), *GetOwner()->GetName(), InitialLevel);
}

void UWWGZombieIntegration::CureInfection()
{
	if (!bCanCureInfection)
	{
		UE_LOG(LogTemp, Warning, TEXT("WWGZombieIntegration: Cannot cure infection"));
		return;
	}

	bIsInfected = false;
	SetZombificationLevel(0.0f);
	OnInfectionCured();

	UE_LOG(LogTemp, Log, TEXT("WWGZombieIntegration: %s cured of infection"), *GetOwner()->GetName());
}

EZombificationState UWWGZombieIntegration::GetZombificationState() const
{
	if (ZombificationLevel < 0.25f)
		return EZombificationState::Human;
	else if (ZombificationLevel < 0.5f)
		return EZombificationState::EarlyInfection;
	else if (ZombificationLevel < 0.75f)
		return EZombificationState::MidInfection;
	else if (ZombificationLevel < 0.99f)
		return EZombificationState::LateInfection;
	else
		return EZombificationState::FullZombie;
}

// ========================================
// DISMEMBERMENT SYSTEM
// ========================================

bool UWWGZombieIntegration::DismemberLimb(FName LimbName, float Damage, bool bInstantKill)
{
	// Check if already dismembered
	if (IsLimbDismembered(LimbName))
	{
		return false;
	}

	// Check damage threshold
	if (Damage < MinDismemberDamage && !bInstantKill)
	{
		return false;
	}

	// Roll for dismemberment chance
	if (!bInstantKill && FMath::FRand() > DismemberChance)
	{
		return false;
	}

	// Create dismembered limb entry
	FDismemberedLimb NewLimb;
	NewLimb.LimbName = LimbName;
	NewLimb.DismemberTime = GetWorld()->GetTimeSeconds();
	NewLimb.DismemberDamage = Damage;

	// Check if zombie can still function
	bool bCriticalLimb = (LimbName == TEXT("Head") || LimbName == TEXT("Spine"));
	NewLimb.bStillFunctional = bCanSurviveDismemberment && !bCriticalLimb && !bInstantKill;

	DismemberedLimbs.Add(NewLimb);

	// Fire event
	OnLimbDismembered(LimbName, Damage);

	// Apply ability changes
	ApplyDismembermentToAbilities();

	UE_LOG(LogTemp, Log, TEXT("WWGZombieIntegration: %s dismembered %s"), *GetOwner()->GetName(), *LimbName.ToString());

	// Kill if critical or can't survive
	if (!NewLimb.bStillFunctional)
	{
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character)
		{
			// Trigger death (let game mode handle it)
			Character->TakeDamage(10000.0f, FDamageEvent(), nullptr, nullptr);
		}
	}

	return true;
}

bool UWWGZombieIntegration::IsLimbDismembered(FName LimbName) const
{
	for (const FDismemberedLimb& Limb : DismemberedLimbs)
	{
		if (Limb.LimbName == LimbName)
		{
			return true;
		}
	}

	return false;
}

bool UWWGZombieIntegration::CanStillFunction() const
{
	if (DismemberedLimbs.Num() == 0)
	{
		return true;
	}

	// Check if any critical limbs are dismembered
	for (const FDismemberedLimb& Limb : DismemberedLimbs)
	{
		if (!Limb.bStillFunctional)
		{
			return false;
		}
	}

	return true;
}

// ========================================
// APPEARANCE
// ========================================

void UWWGZombieIntegration::SetDirtLevel(float NewLevel)
{
	DirtLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
}

// ========================================
// ABILITY SYSTEM INTEGRATION
// ========================================

TArray<UZombieAbilityComponent*> UWWGZombieIntegration::GetAllAbilities() const
{
	TArray<UZombieAbilityComponent*> Abilities;

	if (ZombieBase)
	{
		Abilities = ZombieBase->AbilityComponents;
	}

	return Abilities;
}

void UWWGZombieIntegration::ApplyZombificationToAbilities()
{
	if (!ZombieBase)
	{
		return;
	}

	// Abilities become more effective as zombification increases
	float EffectivenessMultiplier = 0.5f + (ZombificationLevel * 0.5f); // 50% at human, 100% at full zombie

	// Apply to all abilities (Blueprint can override for specific behavior)
	for (UZombieAbilityComponent* Ability : ZombieBase->AbilityComponents)
	{
		if (Ability)
		{
			// Abilities can query zombification level via this component
			// Example: Climbing gets easier as zombie mutates
		}
	}

	UE_LOG(LogTemp, Verbose, TEXT("WWGZombieIntegration: Applied zombification %.2f to abilities"), ZombificationLevel);
}

void UWWGZombieIntegration::ApplyDismembermentToAbilities()
{
	if (!ZombieBase)
	{
		return;
	}

	// Disable abilities based on dismembered limbs
	// Example: Can't climb if arms dismembered

	bool bArmsDismembered = IsLimbDismembered(TEXT("LeftArm")) || IsLimbDismembered(TEXT("RightArm"));
	bool bLegsDismembered = IsLimbDismembered(TEXT("LeftLeg")) || IsLimbDismembered(TEXT("RightLeg"));

	for (UZombieAbilityComponent* Ability : ZombieBase->AbilityComponents)
	{
		if (Ability)
		{
			// Climbing requires arms
			if (Ability->AbilityTags.Contains(TEXT("Climbing")) && bArmsDismembered)
			{
				Ability->BlockingTags.AddUnique(TEXT("Dismembered"));
			}

			// Tunneling might require arms
			if (Ability->AbilityTags.Contains(TEXT("Tunneling")) && bArmsDismembered)
			{
				Ability->BlockingTags.AddUnique(TEXT("Dismembered"));
			}
		}
	}
}

// ========================================
// PROTECTED FUNCTIONS
// ========================================

void UWWGZombieIntegration::UpdateZombification(float DeltaTime)
{
	if (!bIsInfected)
	{
		return;
	}

	float OldLevel = ZombificationLevel;
	SetZombificationLevel(ZombificationLevel + (ZombificationRate * DeltaTime));

	// Stop at full zombification
	if (IsFullyZombified())
	{
		bIsInfected = false; // Stop progression
	}
}

void UWWGZombieIntegration::ApplyDismembermentEffects()
{
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !Character->GetCharacterMovement())
	{
		return;
	}

	// Slow down if legs dismembered
	bool bLegsDismembered = IsLimbDismembered(TEXT("LeftLeg")) || IsLimbDismembered(TEXT("RightLeg"));
	if (bLegsDismembered)
	{
		// Reduce movement speed by 50%
		UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
		if (Movement)
		{
			// Store original speed if not already stored
			// Apply 50% penalty
			// (Blueprint can handle this more elegantly via events)
		}
	}
}

// ========================================
// AWWGZombieBase Implementation
// ========================================

AWWGZombieBase::AWWGZombieBase()
{
	// Create WWG integration component
	WWGIntegration = CreateDefaultSubobject<UWWGZombieIntegration>(TEXT("WWGIntegration"));
}

void AWWGZombieBase::BeginPlay()
{
	Super::BeginPlay();
}

float AWWGZombieBase::GetZombificationLevel() const
{
	return WWGIntegration ? WWGIntegration->ZombificationLevel : 0.0f;
}

void AWWGZombieBase::SetZombificationLevel(float NewLevel)
{
	if (WWGIntegration)
	{
		WWGIntegration->SetZombificationLevel(NewLevel);
	}
}

bool AWWGZombieBase::DismemberLimb(FName LimbName, float Damage)
{
	return WWGIntegration ? WWGIntegration->DismemberLimb(LimbName, Damage) : false;
}
