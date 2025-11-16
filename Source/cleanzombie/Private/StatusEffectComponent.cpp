// Copyright Epic Games, Inc. All Rights Reserved.

#include "StatusEffectComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UStatusEffectComponent::UStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateActiveEffects(DeltaTime);
}

// ========================================
// STATUS EFFECT MANAGEMENT
// ========================================

void UStatusEffectComponent::ApplyStatusEffect(EStatusEffectType EffectType, float Strength, float Duration, AActor* Instigator)
{
	if (EffectType == EStatusEffectType::None)
	{
		return;
	}

	FStatusEffect NewEffect = CreateDefaultEffect(EffectType, Strength, Duration, Instigator);
	ApplyCustomStatusEffect(NewEffect);
}

void UStatusEffectComponent::ApplyCustomStatusEffect(const FStatusEffect& Effect)
{
	if (Effect.EffectType == EStatusEffectType::None)
	{
		return;
	}

	// Check if we already have this effect
	bool bFound = false;
	for (FStatusEffect& ActiveEffect : ActiveEffects)
	{
		if (ActiveEffect.EffectType == Effect.EffectType)
		{
			bFound = true;

			// Handle stacking
			if (Effect.bCanStack && ActiveEffect.CurrentStacks < ActiveEffect.MaxStacks)
			{
				ActiveEffect.CurrentStacks++;
				ActiveEffect.Strength += Effect.Strength;
				ActiveEffect.TimeRemaining = FMath::Max(ActiveEffect.TimeRemaining, Effect.Duration);

				if (bShowDebug)
				{
					UE_LOG(LogTemp, Log, TEXT("StatusEffect: Stacked %d effect on %s (Stacks: %d/%d)"),
						(int32)Effect.EffectType, *GetOwner()->GetName(), ActiveEffect.CurrentStacks, ActiveEffect.MaxStacks);
				}
			}
			else
			{
				// Refresh duration
				ActiveEffect.TimeRemaining = FMath::Max(ActiveEffect.TimeRemaining, Effect.Duration);
				ActiveEffect.Strength = FMath::Max(ActiveEffect.Strength, Effect.Strength);

				if (bShowDebug)
				{
					UE_LOG(LogTemp, Log, TEXT("StatusEffect: Refreshed %d effect on %s"),
						(int32)Effect.EffectType, *GetOwner()->GetName());
				}
			}

			break;
		}
	}

	// Add new effect if not found
	if (!bFound)
	{
		FStatusEffect NewEffect = Effect;
		NewEffect.TimeRemaining = Effect.Duration;
		ActiveEffects.Add(NewEffect);

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("StatusEffect: Applied %d effect to %s (Duration: %.1f, Strength: %.1f)"),
				(int32)Effect.EffectType, *GetOwner()->GetName(), Effect.Duration, Effect.Strength);
		}

		OnStatusEffectApplied(NewEffect);
	}
}

void UStatusEffectComponent::RemoveStatusEffect(EStatusEffectType EffectType)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
	{
		if (ActiveEffects[i].EffectType == EffectType)
		{
			ActiveEffects.RemoveAt(i);
			OnStatusEffectRemoved(EffectType);

			if (bShowDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("StatusEffect: Removed %d effect from %s"),
					(int32)EffectType, *GetOwner()->GetName());
			}

			break;
		}
	}
}

void UStatusEffectComponent::ClearAllStatusEffects()
{
	for (const FStatusEffect& Effect : ActiveEffects)
	{
		OnStatusEffectRemoved(Effect.EffectType);
	}

	ActiveEffects.Empty();

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Cleared all effects from %s"), *GetOwner()->GetName());
	}
}

bool UStatusEffectComponent::HasStatusEffect(EStatusEffectType EffectType) const
{
	for (const FStatusEffect& Effect : ActiveEffects)
	{
		if (Effect.EffectType == EffectType)
		{
			return true;
		}
	}

	return false;
}

FStatusEffect UStatusEffectComponent::GetStatusEffect(EStatusEffectType EffectType) const
{
	for (const FStatusEffect& Effect : ActiveEffects)
	{
		if (Effect.EffectType == EffectType)
		{
			return Effect;
		}
	}

	return FStatusEffect();
}

float UStatusEffectComponent::GetMovementSpeedMultiplier() const
{
	float Multiplier = 1.0f;

	for (const FStatusEffect& Effect : ActiveEffects)
	{
		switch (Effect.EffectType)
		{
		case EStatusEffectType::Slowing:
			Multiplier *= (1.0f - Effect.Strength);
			break;

		case EStatusEffectType::Stun:
			return 0.0f; // Complete immobilization
			break;

		default:
			break;
		}
	}

	return FMath::Clamp(Multiplier, 0.0f, 1.0f);
}

float UStatusEffectComponent::GetDamageOutputMultiplier() const
{
	float Multiplier = 1.0f;

	for (const FStatusEffect& Effect : ActiveEffects)
	{
		if (Effect.EffectType == EStatusEffectType::Weakness)
		{
			Multiplier *= (1.0f - Effect.Strength);
		}
	}

	return FMath::Clamp(Multiplier, 0.0f, 1.0f);
}

bool UStatusEffectComponent::IsStunned() const
{
	return HasStatusEffect(EStatusEffectType::Stun);
}

bool UStatusEffectComponent::IsBlinded() const
{
	return HasStatusEffect(EStatusEffectType::Blinding);
}

// ========================================
// PROTECTED FUNCTIONS
// ========================================

void UStatusEffectComponent::UpdateActiveEffects(float DeltaTime)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
	{
		FStatusEffect& Effect = ActiveEffects[i];

		// Process effect
		ProcessEffect(Effect, DeltaTime);

		// Update time remaining
		Effect.TimeRemaining -= DeltaTime;

		// Remove expired effects
		if (Effect.TimeRemaining <= 0.0f)
		{
			EStatusEffectType ExpiredType = Effect.EffectType;
			ActiveEffects.RemoveAt(i);
			OnStatusEffectRemoved(ExpiredType);

			if (bShowDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("StatusEffect: %d effect expired on %s"),
					(int32)ExpiredType, *GetOwner()->GetName());
			}
		}
	}
}

void UStatusEffectComponent::ProcessEffect(FStatusEffect& Effect, float DeltaTime)
{
	Effect.TimeSinceLastTick += DeltaTime;

	// Process DoT effects
	bool bIsDoTEffect = (Effect.EffectType == EStatusEffectType::Poison ||
		Effect.EffectType == EStatusEffectType::Acid ||
		Effect.EffectType == EStatusEffectType::Fire);

	if (bIsDoTEffect && Effect.TimeSinceLastTick >= Effect.TickInterval)
	{
		ApplyDoTDamage(Effect);
		Effect.TimeSinceLastTick = 0.0f;
	}

	// Process movement speed effects
	if (Effect.EffectType == EStatusEffectType::Slowing ||
		Effect.EffectType == EStatusEffectType::Stun)
	{
		ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
		if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
		{
			float SpeedMultiplier = GetMovementSpeedMultiplier();
			// The game should query this multiplier and apply it
			// This is handled externally by the character's movement logic
		}
	}
}

void UStatusEffectComponent::ApplyDoTDamage(const FStatusEffect& Effect)
{
	if (Effect.DamagePerTick <= 0.0f)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Calculate total damage (base damage * strength * stacks)
	float TotalDamage = Effect.DamagePerTick * Effect.Strength * Effect.CurrentStacks;

	// Apply damage
	UGameplayStatics::ApplyDamage(
		Owner,
		TotalDamage,
		Effect.Instigator ? Effect.Instigator->GetInstigatorController() : nullptr,
		Effect.Instigator,
		UDamageType::StaticClass()
	);

	OnDoTDamageDealt(TotalDamage, Effect.EffectType, Effect.Instigator);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Applied %.1f %d damage to %s"),
			TotalDamage, (int32)Effect.EffectType, *Owner->GetName());
	}
}

FStatusEffect UStatusEffectComponent::CreateDefaultEffect(EStatusEffectType EffectType, float Strength, float Duration, AActor* Instigator)
{
	FStatusEffect Effect;
	Effect.EffectType = EffectType;
	Effect.Strength = Strength;
	Effect.Duration = Duration;
	Effect.TimeRemaining = Duration;
	Effect.Instigator = Instigator;

	// Set defaults based on type
	switch (EffectType)
	{
	case EStatusEffectType::Poison:
		Effect.DamagePerTick = DefaultPoisonDamage * Strength;
		Effect.TickInterval = 1.0f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("Poison");
		break;

	case EStatusEffectType::Acid:
		Effect.DamagePerTick = DefaultAcidDamage * Strength;
		Effect.TickInterval = 0.5f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 5;
		Effect.EffectTag = TEXT("Acid");
		break;

	case EStatusEffectType::Fire:
		Effect.DamagePerTick = DefaultFireDamage * Strength;
		Effect.TickInterval = 0.5f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("Fire");
		break;

	case EStatusEffectType::Slowing:
		Effect.Strength = DefaultSlowPercentage * Strength;
		Effect.bCanStack = true;
		Effect.MaxStacks = 2;
		Effect.EffectTag = TEXT("Slow");
		break;

	case EStatusEffectType::Blinding:
		Effect.Strength = DefaultBlindStrength * Strength;
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Blind");
		break;

	case EStatusEffectType::Stun:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Stun");
		break;

	case EStatusEffectType::Weakness:
		Effect.Strength = 0.5f * Strength; // 50% damage reduction
		Effect.bCanStack = true;
		Effect.MaxStacks = 2;
		Effect.EffectTag = TEXT("Weakness");
		break;

	default:
		break;
	}

	return Effect;
}

// ========================================
// EVENTS
// ========================================

void UStatusEffectComponent::OnStatusEffectApplied_Implementation(const FStatusEffect& Effect)
{
	// Override in Blueprint
}

void UStatusEffectComponent::OnStatusEffectRemoved_Implementation(EStatusEffectType EffectType)
{
	// Override in Blueprint
}

void UStatusEffectComponent::OnDoTDamageDealt_Implementation(float Damage, EStatusEffectType EffectType, AActor* Instigator)
{
	// Override in Blueprint
}
