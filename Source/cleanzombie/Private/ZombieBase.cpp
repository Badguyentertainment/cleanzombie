// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZombieBase.h"
#include "ZombieAbilityComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AZombieBase::AZombieBase()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication
	bReplicates = true;
	SetReplicateMovement(true);

	// Default configuration
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}

void AZombieBase::BeginPlay()
{
	Super::BeginPlay();

	// Initialize from config if specified
	if (!ConfigRowName.IsNone() && ZombieConfigTable)
	{
		InitializeFromConfig(ConfigRowName);
	}
	else
	{
		// Initialize abilities from current config
		for (TSubclassOf<UZombieAbilityComponent> AbilityClass : CurrentConfig.AbilityClasses)
		{
			AddAbility(AbilityClass);
		}
	}
}

void AZombieBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShowDebugInfo)
	{
		DrawDebugInfo();
	}
}

void AZombieBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZombieBase, ZombieVariant);
	DOREPLIFETIME(AZombieBase, CurrentHealth);
	DOREPLIFETIME(AZombieBase, MaxHealth);
	DOREPLIFETIME(AZombieBase, bIsAlive);
	DOREPLIFETIME(AZombieBase, CurrentTarget);
}

// ========================================
// ZOMBIE CONFIGURATION
// ========================================

void AZombieBase::InitializeFromConfig(FName RowName)
{
	if (!ZombieConfigTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZombieBase: No config table assigned!"));
		return;
	}

	FZombieConfigData* ConfigRow = ZombieConfigTable->FindRow<FZombieConfigData>(RowName, TEXT("ZombieConfig"));
	if (!ConfigRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("ZombieBase: Config row '%s' not found!"), *RowName.ToString());
		return;
	}

	CurrentConfig = *ConfigRow;
	ConfigRowName = RowName;

	ApplyConfiguration(CurrentConfig);
}

void AZombieBase::InitializeFromVariant(EZombieVariant Variant)
{
	ZombieVariant = Variant;

	// Find config row matching this variant
	if (ZombieConfigTable)
	{
		TArray<FName> RowNames = ZombieConfigTable->GetRowNames();
		for (const FName& RowName : RowNames)
		{
			FZombieConfigData* ConfigRow = ZombieConfigTable->FindRow<FZombieConfigData>(RowName, TEXT("ZombieConfig"));
			if (ConfigRow && ConfigRow->VariantType == Variant)
			{
				InitializeFromConfig(RowName);
				return;
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ZombieBase: No config found for variant %d"), (int32)Variant);
}

void AZombieBase::ApplyConfiguration(const FZombieConfigData& Config)
{
	// Apply stats
	MaxHealth = Config.MaxHealth;
	CurrentHealth = MaxHealth;
	ZombieVariant = Config.VariantType;

	// Apply movement speed
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = Config.MovementSpeed;
	}

	// Apply mesh
	if (Config.ZombieMesh && GetMesh())
	{
		GetMesh()->SetSkeletalMesh(Config.ZombieMesh);
	}

	// Apply animation blueprint
	if (Config.AnimationBlueprint && GetMesh())
	{
		GetMesh()->SetAnimInstanceClass(Config.AnimationBlueprint);
	}

	// Apply scale
	SetActorScale3D(Config.ScaleMultiplier);

	// Add abilities
	for (TSubclassOf<UZombieAbilityComponent> AbilityClass : Config.AbilityClasses)
	{
		if (AbilityClass && !HasAbility(AbilityClass))
		{
			AddAbility(AbilityClass);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ZombieBase: Applied config '%s' - Health: %.0f, Speed: %.0f, Abilities: %d"),
		*Config.DisplayName.ToString(), MaxHealth, Config.MovementSpeed, AbilityComponents.Num());
}

// ========================================
// ZOMBIE STATS
// ========================================

float AZombieBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!bIsAlive)
	{
		return 0.0f;
	}

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth -= ActualDamage;

	// Notify abilities
	NotifyAbilitiesOfDamage(ActualDamage, DamageCauser);

	// Call damage event
	OnDamaged(ActualDamage, DamageCauser);

	// Check for death
	if (CurrentHealth <= 0.0f)
	{
		Die(DamageCauser);
	}

	return ActualDamage;
}

void AZombieBase::Die(AActor* Killer)
{
	if (!bIsAlive)
	{
		return;
	}

	bIsAlive = false;
	CurrentHealth = 0.0f;

	// Deactivate all abilities
	for (UZombieAbilityComponent* Ability : AbilityComponents)
	{
		if (Ability && Ability->bIsActive)
		{
			Ability->DeactivateAbility();
		}
	}

	// Call death event
	OnDeath(Killer);

	// Disable collision
	SetActorEnableCollision(false);

	// TODO: Trigger death animation, ragdoll, gibbing, etc.
}

void AZombieBase::Heal(float Amount)
{
	if (!bIsAlive)
	{
		return;
	}

	CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
}

// ========================================
// ABILITY SYSTEM
// ========================================

UZombieAbilityComponent* AZombieBase::AddAbility(TSubclassOf<UZombieAbilityComponent> AbilityClass)
{
	if (!AbilityClass)
	{
		return nullptr;
	}

	// Create and attach ability component
	UZombieAbilityComponent* NewAbility = NewObject<UZombieAbilityComponent>(this, AbilityClass);
	if (NewAbility)
	{
		NewAbility->RegisterComponent();
		AbilityComponents.Add(NewAbility);

		UE_LOG(LogTemp, Log, TEXT("ZombieBase: Added ability '%s'"), *NewAbility->AbilityName.ToString());

		return NewAbility;
	}

	return nullptr;
}

void AZombieBase::RemoveAbility(UZombieAbilityComponent* Ability)
{
	if (!Ability)
	{
		return;
	}

	// Deactivate if active
	if (Ability->bIsActive)
	{
		Ability->DeactivateAbility();
	}

	// Remove from list
	AbilityComponents.Remove(Ability);

	// Destroy component
	Ability->DestroyComponent();
}

UZombieAbilityComponent* AZombieBase::GetAbilityByClass(TSubclassOf<UZombieAbilityComponent> AbilityClass) const
{
	for (UZombieAbilityComponent* Ability : AbilityComponents)
	{
		if (Ability && Ability->IsA(AbilityClass))
		{
			return Ability;
		}
	}
	return nullptr;
}

TArray<UZombieAbilityComponent*> AZombieBase::GetAbilitiesByClass(TSubclassOf<UZombieAbilityComponent> AbilityClass) const
{
	TArray<UZombieAbilityComponent*> FoundAbilities;

	for (UZombieAbilityComponent* Ability : AbilityComponents)
	{
		if (Ability && Ability->IsA(AbilityClass))
		{
			FoundAbilities.Add(Ability);
		}
	}

	return FoundAbilities;
}

bool AZombieBase::HasAbility(TSubclassOf<UZombieAbilityComponent> AbilityClass) const
{
	return GetAbilityByClass(AbilityClass) != nullptr;
}

bool AZombieBase::ActivateAbility(TSubclassOf<UZombieAbilityComponent> AbilityClass)
{
	UZombieAbilityComponent* Ability = GetAbilityByClass(AbilityClass);
	if (Ability)
	{
		return Ability->ActivateAbility();
	}
	return false;
}

void AZombieBase::DeactivateAbility(TSubclassOf<UZombieAbilityComponent> AbilityClass)
{
	UZombieAbilityComponent* Ability = GetAbilityByClass(AbilityClass);
	if (Ability)
	{
		Ability->DeactivateAbility();
	}
}

TArray<UZombieAbilityComponent*> AZombieBase::GetActiveAbilities() const
{
	TArray<UZombieAbilityComponent*> ActiveAbilities;

	for (UZombieAbilityComponent* Ability : AbilityComponents)
	{
		if (Ability && Ability->bIsActive)
		{
			ActiveAbilities.Add(Ability);
		}
	}

	return ActiveAbilities;
}

// ========================================
// TARGETING
// ========================================

void AZombieBase::SetTarget(AActor* NewTarget)
{
	AActor* OldTarget = CurrentTarget;
	CurrentTarget = NewTarget;

	// Notify abilities
	if (NewTarget && NewTarget != OldTarget)
	{
		NotifyAbilitiesOfTargetDetected(NewTarget);
		OnTargetDetected(NewTarget);

		// Update ability targets
		for (UZombieAbilityComponent* Ability : AbilityComponents)
		{
			if (Ability)
			{
				Ability->SetTarget(NewTarget);
			}
		}
	}

	if (OldTarget && OldTarget != NewTarget)
	{
		NotifyAbilitiesOfTargetLost(OldTarget);
		OnTargetLost(OldTarget);
	}
}

void AZombieBase::ClearTarget()
{
	SetTarget(nullptr);
}

bool AZombieBase::HasValidTarget() const
{
	return CurrentTarget != nullptr && IsValid(CurrentTarget);
}

// ========================================
// EVENTS
// ========================================

void AZombieBase::OnDamaged_Implementation(float Damage, AActor* DamageSource)
{
	// Override in Blueprint
}

void AZombieBase::OnDeath_Implementation(AActor* Killer)
{
	// Override in Blueprint
}

void AZombieBase::OnTargetDetected_Implementation(AActor* DetectedTarget)
{
	// Override in Blueprint
}

void AZombieBase::OnTargetLost_Implementation(AActor* LostTarget)
{
	// Override in Blueprint
}

void AZombieBase::OnKilledTarget_Implementation(AActor* VictimActor)
{
	// Override in Blueprint
}

// ========================================
// ABILITY NOTIFICATIONS
// ========================================

void AZombieBase::NotifyAbilitiesOfDamage(float Damage, AActor* DamageSource)
{
	for (UZombieAbilityComponent* Ability : AbilityComponents)
	{
		if (Ability)
		{
			Ability->OnZombieDamaged(Damage, DamageSource);
		}
	}
}

void AZombieBase::NotifyAbilitiesOfKill(AActor* VictimActor)
{
	for (UZombieAbilityComponent* Ability : AbilityComponents)
	{
		if (Ability)
		{
			Ability->OnZombieKilledTarget(VictimActor);
		}
	}
}

void AZombieBase::NotifyAbilitiesOfTargetDetected(AActor* DetectedActor)
{
	for (UZombieAbilityComponent* Ability : AbilityComponents)
	{
		if (Ability)
		{
			Ability->OnZombieDetectedTarget(DetectedActor);
		}
	}
}

void AZombieBase::NotifyAbilitiesOfTargetLost(AActor* LostActor)
{
	for (UZombieAbilityComponent* Ability : AbilityComponents)
	{
		if (Ability)
		{
			Ability->OnZombieLostTarget(LostActor);
		}
	}
}

// ========================================
// DEBUG
// ========================================

void AZombieBase::DrawDebugInfo()
{
	if (!GetWorld())
	{
		return;
	}

	FVector Location = GetActorLocation() + FVector(0, 0, 120);

	// Draw zombie info
	FString ZombieInfo = FString::Printf(
		TEXT("%s\nHealth: %.0f/%.0f\nAbilities: %d Active"),
		*CurrentConfig.DisplayName.ToString(),
		CurrentHealth,
		MaxHealth,
		GetActiveAbilities().Num()
	);

	DrawDebugString(GetWorld(), Location, ZombieInfo, nullptr, FColor::White, 0.0f, true);

	// Draw target line
	if (CurrentTarget)
	{
		DrawDebugLine(
			GetWorld(),
			GetActorLocation(),
			CurrentTarget->GetActorLocation(),
			FColor::Red,
			false,
			0.0f,
			0,
			2.0f
		);
	}

	// Draw abilities
	for (int32 i = 0; i < AbilityComponents.Num(); ++i)
	{
		if (AbilityComponents[i] && AbilityComponents[i]->bShowDebug)
		{
			AbilityComponents[i]->DrawDebugInfo();
		}
	}
}
