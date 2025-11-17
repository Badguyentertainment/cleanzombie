// Copyright Epic Games, Inc. All Rights Reserved.

#include "StatusEffectComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"

UStatusEffectComponent::UStatusEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

	SetupDefaultCombos();
}

void UStatusEffectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UStatusEffectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cleanup all visual effects
	for (FStatusEffect& Effect : ActiveEffects)
	{
		CleanupVisualFeedback(Effect);
	}

	Super::EndPlay(EndPlayReason);
}

void UStatusEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateActiveEffects(DeltaTime);

	// Check for combos periodically
	if (bEnableCombos && ActiveEffects.Num() >= 2)
	{
		CheckForCombos();
	}
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

	// Check immunity
	if (IsImmuneTo(EffectType))
	{
		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("StatusEffect: %s is immune to %d effect"),
				*GetOwner()->GetName(), (int32)EffectType);
		}
		return;
	}

	FStatusEffect NewEffect = CreateDefaultEffect(EffectType, Strength, Duration, Instigator);

	// Apply resistance
	ApplyResistance(NewEffect);

	ApplyCustomStatusEffect(NewEffect);
}

void UStatusEffectComponent::ApplyCustomStatusEffect(const FStatusEffect& Effect)
{
	if (Effect.EffectType == EStatusEffectType::None)
	{
		return;
	}

	// Check immunity
	if (IsImmuneTo(Effect.EffectType))
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
		NewEffect.Severity = CalculateSeverity(Effect.Strength);
		ActiveEffects.Add(NewEffect);

		// Spawn visual feedback
		if (bEnableVisuals)
		{
			SpawnVisualFeedback(ActiveEffects.Last());
		}

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("StatusEffect: Applied %d effect to %s (Duration: %.1f, Strength: %.1f, Severity: %d)"),
				(int32)Effect.EffectType, *GetOwner()->GetName(), Effect.Duration, Effect.Strength, (int32)NewEffect.Severity);
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
			CleanupVisualFeedback(ActiveEffects[i]);
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
		CleanupVisualFeedback(const_cast<FStatusEffect&>(Effect));
		OnStatusEffectRemoved(Effect.EffectType);
	}

	ActiveEffects.Empty();
	CurrentShieldAmount = 0.0f;

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

		case EStatusEffectType::Frozen:
			Multiplier *= (1.0f - Effect.Strength);
			break;

		case EStatusEffectType::Stun:
		case EStatusEffectType::Rooted:
			return 0.0f; // Complete immobilization
			break;

		case EStatusEffectType::Haste:
			Multiplier *= (1.0f + Effect.Strength);
			break;

		default:
			break;
		}
	}

	return FMath::Clamp(Multiplier, 0.0f, 3.0f);
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
		else if (Effect.EffectType == EStatusEffectType::DamageBoost)
		{
			Multiplier *= (1.0f + Effect.Strength);
		}
		else if (Effect.EffectType == EStatusEffectType::Disarmed)
		{
			return 0.0f;
		}
	}

	return FMath::Clamp(Multiplier, 0.0f, 5.0f);
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
// RESISTANCE & IMMUNITY
// ========================================

void UStatusEffectComponent::AddResistance(EStatusEffectType EffectType, float ResistanceAmount, bool bReducesDuration, bool bReducesStrength)
{
	// Check if resistance already exists
	for (FEffectResistance& Resistance : Resistances)
	{
		if (Resistance.EffectType == EffectType)
		{
			Resistance.ResistanceAmount = FMath::Max(Resistance.ResistanceAmount, ResistanceAmount);
			Resistance.bReducesDuration = bReducesDuration;
			Resistance.bReducesStrength = bReducesStrength;
			return;
		}
	}

	// Add new resistance
	FEffectResistance NewResistance;
	NewResistance.EffectType = EffectType;
	NewResistance.ResistanceAmount = ResistanceAmount;
	NewResistance.bReducesDuration = bReducesDuration;
	NewResistance.bReducesStrength = bReducesStrength;
	Resistances.Add(NewResistance);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Added %.0f%% resistance to %d effect on %s"),
			ResistanceAmount * 100.0f, (int32)EffectType, *GetOwner()->GetName());
	}
}

void UStatusEffectComponent::RemoveResistance(EStatusEffectType EffectType)
{
	for (int32 i = Resistances.Num() - 1; i >= 0; i--)
	{
		if (Resistances[i].EffectType == EffectType)
		{
			Resistances.RemoveAt(i);
			break;
		}
	}
}

float UStatusEffectComponent::GetResistance(EStatusEffectType EffectType) const
{
	for (const FEffectResistance& Resistance : Resistances)
	{
		if (Resistance.EffectType == EffectType)
		{
			return Resistance.ResistanceAmount;
		}
	}

	return 0.0f;
}

bool UStatusEffectComponent::IsImmuneTo(EStatusEffectType EffectType) const
{
	// Check for Invulnerable buff
	if (HasStatusEffect(EStatusEffectType::Invulnerable))
	{
		return true;
	}

	// Check resistances
	return GetResistance(EffectType) >= 1.0f;
}

// ========================================
// CLEANSING & CURING
// ========================================

void UStatusEffectComponent::CleanseEffect(EStatusEffectType EffectType)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
	{
		if (ActiveEffects[i].EffectType == EffectType && ActiveEffects[i].bCanBeCleansed)
		{
			CleanupVisualFeedback(ActiveEffects[i]);
			ActiveEffects.RemoveAt(i);
			OnEffectCleansed(EffectType);

			if (bShowDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("StatusEffect: Cleansed %d effect from %s"),
					(int32)EffectType, *GetOwner()->GetName());
			}

			break;
		}
	}
}

void UStatusEffectComponent::CleanseAllDebuffs()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
	{
		if (IsDebuff(ActiveEffects[i].EffectType) && ActiveEffects[i].bCanBeCleansed)
		{
			EStatusEffectType EffectType = ActiveEffects[i].EffectType;
			CleanupVisualFeedback(ActiveEffects[i]);
			ActiveEffects.RemoveAt(i);
			OnEffectCleansed(EffectType);
		}
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Cleansed all debuffs from %s"), *GetOwner()->GetName());
	}
}

void UStatusEffectComponent::CleanseAllBuffs()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
	{
		if (IsBuff(ActiveEffects[i].EffectType) && ActiveEffects[i].bCanBeCleansed)
		{
			EStatusEffectType EffectType = ActiveEffects[i].EffectType;
			CleanupVisualFeedback(ActiveEffects[i]);
			ActiveEffects.RemoveAt(i);
			OnEffectCleansed(EffectType);
		}
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Cleansed all buffs from %s"), *GetOwner()->GetName());
	}
}

void UStatusEffectComponent::CleanseAll()
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
	{
		if (ActiveEffects[i].bCanBeCleansed)
		{
			EStatusEffectType EffectType = ActiveEffects[i].EffectType;
			CleanupVisualFeedback(ActiveEffects[i]);
			ActiveEffects.RemoveAt(i);
			OnEffectCleansed(EffectType);
		}
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Cleansed all cleansable effects from %s"), *GetOwner()->GetName());
	}
}

void UStatusEffectComponent::CleanseBySeverity(EEffectSeverity MaxSeverity)
{
	for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
	{
		if (ActiveEffects[i].Severity <= MaxSeverity && ActiveEffects[i].bCanBeCleansed)
		{
			EStatusEffectType EffectType = ActiveEffects[i].EffectType;
			CleanupVisualFeedback(ActiveEffects[i]);
			ActiveEffects.RemoveAt(i);
			OnEffectCleansed(EffectType);
		}
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Cleansed effects up to severity %d from %s"),
			(int32)MaxSeverity, *GetOwner()->GetName());
	}
}

// ========================================
// COMBOS & SYNERGIES
// ========================================

void UStatusEffectComponent::CheckForCombos()
{
	if (!bEnableCombos || ActiveEffects.Num() < 2)
	{
		return;
	}

	// Check all pairs of active effects
	for (int32 i = 0; i < ActiveEffects.Num(); i++)
	{
		for (int32 j = i + 1; j < ActiveEffects.Num(); j++)
		{
			FEffectCombo* Combo = FindCombo(ActiveEffects[i].EffectType, ActiveEffects[j].EffectType);
			if (Combo)
			{
				TriggerCombo(ActiveEffects[i].EffectType, ActiveEffects[j].EffectType);
			}
		}
	}
}

bool UStatusEffectComponent::TriggerCombo(EStatusEffectType EffectA, EStatusEffectType EffectB)
{
	FEffectCombo* Combo = FindCombo(EffectA, EffectB);
	if (!Combo)
	{
		return false;
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Triggered combo %d + %d = %d on %s"),
			(int32)EffectA, (int32)EffectB, (int32)Combo->ResultEffect, *GetOwner()->GetName());
	}

	// Apply bonus damage
	if (Combo->BonusDamage > 0.0f)
	{
		UGameplayStatics::ApplyDamage(
			GetOwner(),
			Combo->BonusDamage,
			nullptr,
			nullptr,
			UDamageType::StaticClass()
		);
	}

	// Apply result effect
	if (Combo->ResultEffect != EStatusEffectType::None)
	{
		ApplyStatusEffect(Combo->ResultEffect, 1.0f, 5.0f, nullptr);
	}

	// Apply AoE damage if combo has radius
	if (Combo->ComboRadius > 0.0f)
	{
		TArray<AActor*> IgnoreActors;
		IgnoreActors.Add(GetOwner());

		UGameplayStatics::ApplyRadialDamage(
			GetWorld(),
			Combo->BonusDamage * 0.5f,
			GetOwner()->GetActorLocation(),
			Combo->ComboRadius,
			UDamageType::StaticClass(),
			IgnoreActors,
			nullptr,
			nullptr,
			false
		);
	}

	// Consume effects if specified
	if (Combo->bConsumeBothEffects)
	{
		RemoveStatusEffect(EffectA);
		RemoveStatusEffect(EffectB);
	}

	OnComboTriggered(*Combo);

	return true;
}

// ========================================
// CONTAGION & SPREADING
// ========================================

void UStatusEffectComponent::SpreadContagiousEffects()
{
	if (!bEnableContagion)
	{
		return;
	}

	for (FStatusEffect& Effect : ActiveEffects)
	{
		if (!Effect.bIsContagious)
		{
			continue;
		}

		Effect.TimeSinceLastContagion += GetWorld()->GetDeltaSeconds();

		if (Effect.TimeSinceLastContagion >= Effect.ContagionTickInterval)
		{
			Effect.TimeSinceLastContagion = 0.0f;

			// Find nearby actors
			TArray<FHitResult> HitResults;
			FCollisionShape SphereShape = FCollisionShape::MakeSphere(Effect.ContagionRadius);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(GetOwner());

			GetWorld()->SweepMultiByChannel(
				HitResults,
				GetOwner()->GetActorLocation(),
				GetOwner()->GetActorLocation(),
				FQuat::Identity,
				ECC_Pawn,
				SphereShape,
				QueryParams
			);

			for (const FHitResult& Hit : HitResults)
			{
				if (FMath::FRand() > Effect.ContagionChance)
				{
					continue;
				}

				AActor* TargetActor = Hit.GetActor();
				if (!TargetActor)
				{
					continue;
				}

				UStatusEffectComponent* TargetComponent = TargetActor->FindComponentByClass<UStatusEffectComponent>();
				if (TargetComponent)
				{
					// Spread with reduced strength
					float SpreadStrength = Effect.Strength * 0.7f;
					float SpreadDuration = Effect.Duration * 0.5f;

					TargetComponent->ApplyStatusEffect(Effect.EffectType, SpreadStrength, SpreadDuration, GetOwner());
					OnEffectSpread(TargetActor, Effect.EffectType);

					if (bShowDebug)
					{
						UE_LOG(LogTemp, Log, TEXT("StatusEffect: Spread %d effect from %s to %s"),
							(int32)Effect.EffectType, *GetOwner()->GetName(), *TargetActor->GetName());
					}
				}
			}
		}
	}
}

bool UStatusEffectComponent::HasContagiousEffects() const
{
	for (const FStatusEffect& Effect : ActiveEffects)
	{
		if (Effect.bIsContagious)
		{
			return true;
		}
	}

	return false;
}

// ========================================
// BUFFS & HEALING
// ========================================

float UStatusEffectComponent::GetShieldAmount() const
{
	return CurrentShieldAmount;
}

void UStatusEffectComponent::ApplyShield(float ShieldAmount, float Duration)
{
	CurrentShieldAmount += ShieldAmount;

	// Add shield effect
	ApplyStatusEffect(EStatusEffectType::Shielded, ShieldAmount, Duration, nullptr);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Applied %.0f shield to %s (Total: %.0f)"),
			ShieldAmount, *GetOwner()->GetName(), CurrentShieldAmount);
	}
}

float UStatusEffectComponent::AbsorbDamage(float IncomingDamage)
{
	if (CurrentShieldAmount <= 0.0f)
	{
		return IncomingDamage;
	}

	float DamageAbsorbed = FMath::Min(IncomingDamage, CurrentShieldAmount);
	CurrentShieldAmount -= DamageAbsorbed;
	float RemainingDamage = IncomingDamage - DamageAbsorbed;

	OnShieldAbsorbed(DamageAbsorbed, CurrentShieldAmount);

	if (CurrentShieldAmount <= 0.0f)
	{
		RemoveStatusEffect(EStatusEffectType::Shielded);
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Shield absorbed %.0f damage on %s (Remaining shield: %.0f)"),
			DamageAbsorbed, *GetOwner()->GetName(), CurrentShieldAmount);
	}

	return RemainingDamage;
}

float UStatusEffectComponent::GetHealingMultiplier() const
{
	float Multiplier = 1.0f;

	for (const FStatusEffect& Effect : ActiveEffects)
	{
		if (Effect.EffectType == EStatusEffectType::Regeneration)
		{
			Multiplier += Effect.Strength;
		}
		else if (Effect.EffectType == EStatusEffectType::Irradiated)
		{
			Multiplier = 0.0f; // Prevents all healing
		}
	}

	return FMath::Clamp(Multiplier, 0.0f, 3.0f);
}

// ========================================
// PROTECTED FUNCTIONS
// ========================================

void UStatusEffectComponent::UpdateActiveEffects(float DeltaTime)
{
	// Spread contagious effects
	if (bEnableContagion && HasContagiousEffects())
	{
		SpreadContagiousEffects();
	}

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
			CleanupVisualFeedback(Effect);
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
		Effect.EffectType == EStatusEffectType::Fire ||
		Effect.EffectType == EStatusEffectType::Bleeding ||
		Effect.EffectType == EStatusEffectType::Electrified ||
		Effect.EffectType == EStatusEffectType::Corroded ||
		Effect.EffectType == EStatusEffectType::Diseased ||
		Effect.EffectType == EStatusEffectType::Irradiated);

	if (bIsDoTEffect && Effect.TimeSinceLastTick >= Effect.TickInterval)
	{
		ApplyDoTDamage(Effect);
		Effect.TimeSinceLastTick = 0.0f;
	}

	// Process HoT effects
	bool bIsHoTEffect = (Effect.EffectType == EStatusEffectType::Regeneration);

	if (bIsHoTEffect && Effect.TimeSinceLastTick >= Effect.TickInterval)
	{
		ApplyHoTHealing(Effect);
		Effect.TimeSinceLastTick = 0.0f;
	}

	// Process special effects
	if (Effect.EffectType == EStatusEffectType::Frozen)
	{
		// Frozen can shatter on damage - implement in your damage handling
	}
	else if (Effect.EffectType == EStatusEffectType::Electrified)
	{
		// Electrified can chain to nearby actors
		if (FMath::FRand() < 0.2f * DeltaTime) // 20% chance per second
		{
			Effect.bIsContagious = true;
			Effect.ContagionRadius = 400.0f;
			Effect.ContagionChance = 0.5f;
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

	// Bleeding gets worse with movement
	if (Effect.EffectType == EStatusEffectType::Bleeding)
	{
		ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
		if (OwnerCharacter && OwnerCharacter->GetCharacterMovement())
		{
			float Speed = OwnerCharacter->GetCharacterMovement()->Velocity.Size();
			float MovementMultiplier = 1.0f + (Speed / OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed);
			TotalDamage *= MovementMultiplier;
		}
	}

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

void UStatusEffectComponent::ApplyHoTHealing(const FStatusEffect& Effect)
{
	if (Effect.HealingPerTick <= 0.0f)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Calculate total healing (base healing * strength * stacks * healing multiplier)
	float TotalHealing = Effect.HealingPerTick * Effect.Strength * Effect.CurrentStacks * GetHealingMultiplier();

	// Apply healing (implement your own healing logic here)
	// For example, you might have a health component:
	// UHealthComponent* HealthComp = Owner->FindComponentByClass<UHealthComponent>();
	// if (HealthComp) HealthComp->Heal(TotalHealing);

	OnHoTHealingApplied(TotalHealing, Effect.EffectType);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("StatusEffect: Applied %.1f %d healing to %s"),
			TotalHealing, (int32)Effect.EffectType, *Owner->GetName());
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
	// DoT Debuffs
	case EStatusEffectType::Poison:
		Effect.DamagePerTick = DefaultPoisonDamage * Strength;
		Effect.TickInterval = 1.0f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("Poison");
		Effect.Visuals.EffectColor = FLinearColor::Green;
		break;

	case EStatusEffectType::Acid:
		Effect.DamagePerTick = DefaultAcidDamage * Strength;
		Effect.TickInterval = 0.5f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 5;
		Effect.EffectTag = TEXT("Acid");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
		break;

	case EStatusEffectType::Fire:
		Effect.DamagePerTick = DefaultFireDamage * Strength;
		Effect.TickInterval = 0.5f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("Fire");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 0.3f, 0.0f, 1.0f); // Orange
		Effect.bIsContagious = true;
		Effect.ContagionRadius = 200.0f;
		Effect.ContagionChance = 0.15f;
		break;

	case EStatusEffectType::Bleeding:
		Effect.DamagePerTick = DefaultBleedingDamage * Strength;
		Effect.TickInterval = 1.0f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 5;
		Effect.EffectTag = TEXT("Bleeding");
		Effect.Visuals.EffectColor = FLinearColor::Red;
		break;

	case EStatusEffectType::Electrified:
		Effect.DamagePerTick = DefaultElectrifiedDamage * Strength;
		Effect.TickInterval = 0.3f;
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Electrified");
		Effect.Visuals.EffectColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f); // Cyan
		break;

	case EStatusEffectType::Corroded:
		Effect.DamagePerTick = DefaultCorrodedDamage * Strength;
		Effect.TickInterval = 1.5f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("Corroded");
		Effect.Visuals.EffectColor = FLinearColor(0.5f, 0.3f, 0.1f, 1.0f); // Brown
		break;

	case EStatusEffectType::Diseased:
		Effect.DamagePerTick = DefaultDiseasedDamage * Strength;
		Effect.TickInterval = 2.0f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 5;
		Effect.EffectTag = TEXT("Diseased");
		Effect.bIsContagious = true;
		Effect.ContagionRadius = 300.0f;
		Effect.ContagionChance = 0.3f;
		Effect.Visuals.EffectColor = FLinearColor(0.3f, 0.6f, 0.3f, 1.0f); // Sickly green
		break;

	case EStatusEffectType::Irradiated:
		Effect.DamagePerTick = DefaultIrradiatedDamage * Strength;
		Effect.TickInterval = 0.5f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 10;
		Effect.EffectTag = TEXT("Irradiated");
		Effect.Visuals.EffectColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Bright green
		break;

	// Movement Debuffs
	case EStatusEffectType::Slowing:
		Effect.Strength = DefaultSlowPercentage * Strength;
		Effect.bCanStack = true;
		Effect.MaxStacks = 2;
		Effect.EffectTag = TEXT("Slow");
		Effect.Visuals.EffectColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f); // Light blue
		break;

	case EStatusEffectType::Frozen:
		Effect.Strength = DefaultFrozenSlowPercentage * Strength;
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Frozen");
		Effect.Visuals.EffectColor = FLinearColor(0.7f, 0.9f, 1.0f, 1.0f); // Ice blue
		break;

	case EStatusEffectType::Stun:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Stun");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
		break;

	case EStatusEffectType::Rooted:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Rooted");
		Effect.Visuals.EffectColor = FLinearColor(0.5f, 0.3f, 0.1f, 1.0f); // Brown
		break;

	// Combat Debuffs
	case EStatusEffectType::Blinding:
		Effect.Strength = DefaultBlindStrength * Strength;
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Blind");
		Effect.Visuals.EffectColor = FLinearColor::Black;
		break;

	case EStatusEffectType::Weakness:
		Effect.Strength = 0.5f * Strength;
		Effect.bCanStack = true;
		Effect.MaxStacks = 2;
		Effect.EffectTag = TEXT("Weakness");
		Effect.Visuals.EffectColor = FLinearColor(0.5f, 0.0f, 0.5f, 1.0f); // Purple
		break;

	case EStatusEffectType::Vulnerability:
		Effect.Strength = 0.5f * Strength;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("Vulnerability");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 0.5f, 0.5f, 1.0f); // Pink
		break;

	case EStatusEffectType::Disarmed:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Disarmed");
		Effect.Visuals.EffectColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Gray
		break;

	// Special Debuffs
	case EStatusEffectType::Confused:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Confused");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
		break;

	case EStatusEffectType::Cursed:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Cursed");
		Effect.bCanBeCleansed = false;
		Effect.Visuals.EffectColor = FLinearColor(0.1f, 0.0f, 0.1f, 1.0f); // Dark purple
		break;

	// Buffs
	case EStatusEffectType::Regeneration:
		Effect.HealingPerTick = DefaultRegenerationHealing * Strength;
		Effect.TickInterval = 1.0f;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("Regeneration");
		Effect.Visuals.EffectColor = FLinearColor(0.0f, 1.0f, 0.5f, 1.0f); // Green
		break;

	case EStatusEffectType::Shielded:
		Effect.EffectTag = TEXT("Shielded");
		Effect.Visuals.EffectColor = FLinearColor(0.5f, 0.5f, 1.0f, 1.0f); // Blue
		break;

	case EStatusEffectType::Blessed:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Blessed");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f); // White
		break;

	case EStatusEffectType::DamageBoost:
		Effect.Strength = 0.5f * Strength;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("DamageBoost");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 0.5f, 0.0f, 1.0f); // Orange
		break;

	case EStatusEffectType::Haste:
		Effect.Strength = 0.5f * Strength;
		Effect.bCanStack = true;
		Effect.MaxStacks = 2;
		Effect.EffectTag = TEXT("Haste");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 1.0f, 0.5f, 1.0f); // Light yellow
		break;

	case EStatusEffectType::Invulnerable:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Invulnerable");
		Effect.Visuals.EffectColor = FLinearColor(1.0f, 0.8f, 0.0f, 1.0f); // Gold
		break;

	case EStatusEffectType::Invisible:
		Effect.bCanStack = false;
		Effect.EffectTag = TEXT("Invisible");
		Effect.Visuals.EffectColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.3f); // Translucent gray
		break;

	case EStatusEffectType::Fortified:
		Effect.Strength = 0.5f * Strength;
		Effect.bCanStack = true;
		Effect.MaxStacks = 3;
		Effect.EffectTag = TEXT("Fortified");
		Effect.Visuals.EffectColor = FLinearColor(0.7f, 0.7f, 0.7f, 1.0f); // Gray
		break;

	default:
		break;
	}

	return Effect;
}

void UStatusEffectComponent::ApplyResistance(FStatusEffect& Effect) const
{
	for (const FEffectResistance& Resistance : Resistances)
	{
		if (Resistance.EffectType == Effect.EffectType)
		{
			if (Resistance.bReducesStrength)
			{
				Effect.Strength *= (1.0f - Resistance.ResistanceAmount);
			}

			if (Resistance.bReducesDuration)
			{
				Effect.Duration *= (1.0f - Resistance.ResistanceAmount);
				Effect.TimeRemaining = Effect.Duration;
			}

			break;
		}
	}
}

EEffectSeverity UStatusEffectComponent::CalculateSeverity(float Strength) const
{
	if (Strength < 0.34f)
	{
		return EEffectSeverity::Minor;
	}
	else if (Strength < 0.67f)
	{
		return EEffectSeverity::Moderate;
	}
	else if (Strength < 0.90f)
	{
		return EEffectSeverity::Severe;
	}
	else
	{
		return EEffectSeverity::Critical;
	}
}

bool UStatusEffectComponent::IsDebuff(EStatusEffectType EffectType) const
{
	// All debuff types
	return (EffectType == EStatusEffectType::Poison ||
		EffectType == EStatusEffectType::Acid ||
		EffectType == EStatusEffectType::Fire ||
		EffectType == EStatusEffectType::Bleeding ||
		EffectType == EStatusEffectType::Corroded ||
		EffectType == EStatusEffectType::Diseased ||
		EffectType == EStatusEffectType::Irradiated ||
		EffectType == EStatusEffectType::Slowing ||
		EffectType == EStatusEffectType::Frozen ||
		EffectType == EStatusEffectType::Stun ||
		EffectType == EStatusEffectType::Rooted ||
		EffectType == EStatusEffectType::Blinding ||
		EffectType == EStatusEffectType::Weakness ||
		EffectType == EStatusEffectType::Vulnerability ||
		EffectType == EStatusEffectType::Disarmed ||
		EffectType == EStatusEffectType::Electrified ||
		EffectType == EStatusEffectType::Confused ||
		EffectType == EStatusEffectType::Cursed);
}

bool UStatusEffectComponent::IsBuff(EStatusEffectType EffectType) const
{
	// All buff types
	return (EffectType == EStatusEffectType::Regeneration ||
		EffectType == EStatusEffectType::Shielded ||
		EffectType == EStatusEffectType::Blessed ||
		EffectType == EStatusEffectType::DamageBoost ||
		EffectType == EStatusEffectType::Haste ||
		EffectType == EStatusEffectType::Invulnerable ||
		EffectType == EStatusEffectType::Invisible ||
		EffectType == EStatusEffectType::Fortified);
}

void UStatusEffectComponent::SpawnVisualFeedback(FStatusEffect& Effect)
{
	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	// Spawn Niagara effect (preferred)
	if (Effect.Visuals.NiagaraEffect)
	{
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			Effect.Visuals.NiagaraEffect,
			Owner->GetRootComponent(),
			Effect.Visuals.AttachSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}
	// Spawn particle effect (legacy)
	else if (Effect.Visuals.ParticleEffect)
	{
		Effect.SpawnedParticle = UGameplayStatics::SpawnEmitterAttached(
			Effect.Visuals.ParticleEffect,
			Owner->GetRootComponent(),
			Effect.Visuals.AttachSocketName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// Play sound
	if (Effect.Visuals.ApplySound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), Effect.Visuals.ApplySound, Owner->GetActorLocation());
	}

	// Start looping sound
	if (Effect.Visuals.LoopSound)
	{
		Effect.SpawnedAudio = UGameplayStatics::SpawnSoundAttached(
			Effect.Visuals.LoopSound,
			Owner->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::SnapToTarget,
			true
		);
	}
}

void UStatusEffectComponent::CleanupVisualFeedback(FStatusEffect& Effect)
{
	if (Effect.SpawnedParticle)
	{
		Effect.SpawnedParticle->DestroyComponent();
		Effect.SpawnedParticle = nullptr;
	}

	if (Effect.SpawnedAudio)
	{
		Effect.SpawnedAudio->Stop();
		Effect.SpawnedAudio->DestroyComponent();
		Effect.SpawnedAudio = nullptr;
	}
}

void UStatusEffectComponent::SetupDefaultCombos()
{
	// Fire + Acid = Explosion
	FEffectCombo FireAcidCombo;
	FireAcidCombo.EffectA = EStatusEffectType::Fire;
	FireAcidCombo.EffectB = EStatusEffectType::Acid;
	FireAcidCombo.ResultEffect = EStatusEffectType::None;
	FireAcidCombo.BonusDamage = 100.0f;
	FireAcidCombo.ComboRadius = 500.0f;
	FireAcidCombo.bConsumeBothEffects = true;
	EffectCombos.Add(FireAcidCombo);

	// Electrified + Water/Slowing = Chain Lightning
	FEffectCombo ElectricWaterCombo;
	ElectricWaterCombo.EffectA = EStatusEffectType::Electrified;
	ElectricWaterCombo.EffectB = EStatusEffectType::Slowing;
	ElectricWaterCombo.ResultEffect = EStatusEffectType::Stun;
	ElectricWaterCombo.BonusDamage = 75.0f;
	ElectricWaterCombo.ComboRadius = 400.0f;
	ElectricWaterCombo.bConsumeBothEffects = false;
	EffectCombos.Add(ElectricWaterCombo);

	// Poison + Weakness = Diseased
	FEffectCombo PoisonWeaknessCombo;
	PoisonWeaknessCombo.EffectA = EStatusEffectType::Poison;
	PoisonWeaknessCombo.EffectB = EStatusEffectType::Weakness;
	PoisonWeaknessCombo.ResultEffect = EStatusEffectType::Diseased;
	PoisonWeaknessCombo.BonusDamage = 0.0f;
	PoisonWeaknessCombo.ComboRadius = 0.0f;
	PoisonWeaknessCombo.bConsumeBothEffects = true;
	EffectCombos.Add(PoisonWeaknessCombo);

	// Frozen + Physical Damage = Shatter (handled externally, but combo exists)
	FEffectCombo FrozenShatterCombo;
	FrozenShatterCombo.EffectA = EStatusEffectType::Frozen;
	FrozenShatterCombo.EffectB = EStatusEffectType::Vulnerability;
	FrozenShatterCombo.ResultEffect = EStatusEffectType::None;
	FrozenShatterCombo.BonusDamage = 150.0f;
	FrozenShatterCombo.ComboRadius = 0.0f;
	FrozenShatterCombo.bConsumeBothEffects = true;
	EffectCombos.Add(FrozenShatterCombo);

	// Bleeding + Fire = Cauterize (stops bleeding but deals bonus damage)
	FEffectCombo BleedingFireCombo;
	BleedingFireCombo.EffectA = EStatusEffectType::Bleeding;
	BleedingFireCombo.EffectB = EStatusEffectType::Fire;
	BleedingFireCombo.ResultEffect = EStatusEffectType::None;
	BleedingFireCombo.BonusDamage = 50.0f;
	BleedingFireCombo.ComboRadius = 0.0f;
	BleedingFireCombo.bConsumeBothEffects = true;
	EffectCombos.Add(BleedingFireCombo);
}

FEffectCombo* UStatusEffectComponent::FindCombo(EStatusEffectType EffectA, EStatusEffectType EffectB)
{
	for (FEffectCombo& Combo : EffectCombos)
	{
		if ((Combo.EffectA == EffectA && Combo.EffectB == EffectB) ||
			(Combo.EffectA == EffectB && Combo.EffectB == EffectA))
		{
			return &Combo;
		}
	}

	return nullptr;
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

void UStatusEffectComponent::OnHoTHealingApplied_Implementation(float Healing, EStatusEffectType EffectType)
{
	// Override in Blueprint
}

void UStatusEffectComponent::OnComboTriggered_Implementation(const FEffectCombo& Combo)
{
	// Override in Blueprint
}

void UStatusEffectComponent::OnEffectCleansed_Implementation(EStatusEffectType EffectType)
{
	// Override in Blueprint
}

void UStatusEffectComponent::OnEffectSpread_Implementation(AActor* TargetActor, EStatusEffectType EffectType)
{
	// Override in Blueprint
}

void UStatusEffectComponent::OnShieldAbsorbed_Implementation(float DamageAbsorbed, float RemainingShield)
{
	// Override in Blueprint
}
