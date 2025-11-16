// Copyright Epic Games, Inc. All Rights Reserved.

#include "MultiTargetingAbility.h"
#include "ZombieBase.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Perception/AIPerceptionComponent.h"

UMultiTargetingAbility::UMultiTargetingAbility()
{
	AbilityName = FText::FromString(TEXT("Multi-Targeting"));
	AbilityDescription = FText::FromString(TEXT("Detect and prioritize multiple target types"));
	AbilityPriority = 10; // Low priority, runs early to set targets for other abilities
	bCanRunConcurrently = true; // Can run with other abilities

	// Initialize default priorities
	TargetTypePriorities.Add(EZombieTargetType::Player, 100.0f);
	TargetTypePriorities.Add(EZombieTargetType::NPC, 80.0f);
	TargetTypePriorities.Add(EZombieTargetType::Barricade, 40.0f);
	TargetTypePriorities.Add(EZombieTargetType::DestructibleObject, 20.0f);
	TargetTypePriorities.Add(EZombieTargetType::Vehicle, 30.0f);

	PriorityLevelMultipliers.Add(ETargetPriority::VeryLow, 0.5f);
	PriorityLevelMultipliers.Add(ETargetPriority::Low, 0.75f);
	PriorityLevelMultipliers.Add(ETargetPriority::Medium, 1.0f);
	PriorityLevelMultipliers.Add(ETargetPriority::High, 1.5f);
	PriorityLevelMultipliers.Add(ETargetPriority::Critical, 2.0f);
}

void UMultiTargetingAbility::BeginPlay()
{
	Super::BeginPlay();

	// Try to get AI perception component if using it
	if (bUseAIPerception && OwnerZombie)
	{
		PerceptionComponent = OwnerZombie->FindComponentByClass<UAIPerceptionComponent>();
	}
}

// ========================================
// ABILITY OVERRIDES
// ========================================

void UMultiTargetingAbility::InitializeAbility_Implementation()
{
	Super::InitializeAbility_Implementation();

	// Perform initial scan
	ScanForTargets();
}

bool UMultiTargetingAbility::ActivateAbility_Implementation()
{
	if (!Super::ActivateAbility_Implementation())
	{
		return false;
	}

	// Start target scanning
	ScanForTargets();

	return true;
}

void UMultiTargetingAbility::DeactivateAbility_Implementation()
{
	ClearCurrentTarget();

	Super::DeactivateAbility_Implementation();
}

void UMultiTargetingAbility::UpdateAbility_Implementation(float DeltaTime)
{
	Super::UpdateAbility_Implementation(DeltaTime);

	TimeSinceLastScan += DeltaTime;
	TimeSinceLastSwitch += DeltaTime;

	// Periodic target scanning
	if (TimeSinceLastScan >= ScanInterval)
	{
		ScanForTargets();
		ProcessDetectedTargets();
		TimeSinceLastScan = 0.0f;
	}

	// Validate current target still valid
	if (BestTarget)
	{
		if (!IsValid(BestTarget) || !IsValidTarget(BestTarget))
		{
			ClearCurrentTarget();
		}
		else
		{
			// Check if target too far
			float Distance = GetDistanceToTarget(BestTarget);
			if (Distance > MaxChaseDistance)
			{
				ClearCurrentTarget();
			}
		}
	}
}

bool UMultiTargetingAbility::CanActivate_Implementation() const
{
	return Super::CanActivate_Implementation() && OwnerZombie != nullptr;
}

// ========================================
// TARGETING FUNCTIONS
// ========================================

void UMultiTargetingAbility::ScanForTargets()
{
	if (!OwnerZombie)
	{
		return;
	}

	DetectedTargets.Empty();

	FVector ZombieLocation = OwnerZombie->GetActorLocation();

	// Method 1: AI Perception (more sophisticated)
	if (bUseAIPerception && PerceptionComponent)
	{
		TArray<AActor*> PerceivedActors;
		PerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

		for (AActor* Actor : PerceivedActors)
		{
			if (IsValidTarget(Actor))
			{
				float Distance = GetDistanceToTarget(Actor);
				if (Distance <= DetectionRange)
				{
					FTargetEvaluationData EvalData = EvaluateTarget(Actor);
					if (EvalData.FinalScore > 0.0f)
					{
						DetectedTargets.Add(EvalData);
					}
				}
			}
		}
	}
	// Method 2: Simple sphere overlap (faster, less accurate)
	else
	{
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwnerZombie);

		GetWorld()->OverlapMultiByChannel(
			OverlapResults,
			ZombieLocation,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeSphere(DetectionRange),
			QueryParams
		);

		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* Actor = Result.GetActor();
			if (IsValidTarget(Actor))
			{
				FTargetEvaluationData EvalData = EvaluateTarget(Actor);
				if (EvalData.FinalScore > 0.0f)
				{
					DetectedTargets.Add(EvalData);
				}
			}
		}
	}

	// Apply group coordination penalties
	if (bEnableGroupCoordination)
	{
		ApplyCoordinationPenalties();
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("MultiTargeting: Found %d valid targets"), DetectedTargets.Num());
	}
}

FTargetEvaluationData UMultiTargetingAbility::EvaluateTarget(AActor* PotentialTarget)
{
	FTargetEvaluationData EvalData;

	if (!IsValidTarget(PotentialTarget) || !OwnerZombie)
	{
		return EvalData;
	}

	TScriptInterface<IZombieTargetInterface> TargetInterface = GetTargetInterface(PotentialTarget);
	if (!TargetInterface)
	{
		return EvalData;
	}

	// Get target info
	EvalData.Target = PotentialTarget;
	EvalData.TargetType = TargetInterface->GetTargetType();
	EvalData.BasePriority = TargetInterface->GetTargetPriority();

	// Check if allowed
	if (!PassesFilters(PotentialTarget, EvalData.TargetType))
	{
		return EvalData;
	}

	// Check if can be targeted
	if (!TargetInterface->CanBeTargeted(OwnerZombie))
	{
		return EvalData;
	}

	// Check if alive
	if (!TargetInterface->IsTargetAlive())
	{
		return EvalData;
	}

	// Calculate distance
	EvalData.Distance = GetDistanceToTarget(PotentialTarget);

	// Check line of sight
	if (bRequireLineOfSight)
	{
		EvalData.bHasLineOfSight = HasLineOfSight(PotentialTarget);
		if (!EvalData.bHasLineOfSight)
		{
			return EvalData; // No LOS = invalid target
		}
	}
	else
	{
		EvalData.bHasLineOfSight = HasLineOfSight(PotentialTarget);
	}

	// Get health percentage
	float CurrentHealth = TargetInterface->GetCurrentHealth();
	float MaxHealth = TargetInterface->GetMaxHealth();
	EvalData.HealthPercentage = MaxHealth > 0 ? (CurrentHealth / MaxHealth) : 1.0f;

	// Calculate final score
	EvalData.FinalScore = CalculateTargetScore(EvalData);

	return EvalData;
}

float UMultiTargetingAbility::CalculateTargetScore(const FTargetEvaluationData& TargetData) const
{
	if (!TargetData.Target)
	{
		return 0.0f;
	}

	// Base score from target type
	float Score = GetTypePriorityScore(TargetData.TargetType);

	// Apply priority level multiplier
	Score *= GetPriorityLevelMultiplier(TargetData.BasePriority);

	// Distance penalty (closer = better)
	if (DistanceWeight > 0.0f && DetectionRange > 0.0f)
	{
		float DistanceFactor = 1.0f - FMath::Clamp(TargetData.Distance / DetectionRange, 0.0f, 1.0f);
		Score += DistanceFactor * DistanceWeight * 100.0f;
	}

	// Health modifier (for players and NPCs, lower health = higher priority)
	if (HealthWeight > 0.0f)
	{
		if (TargetData.TargetType == EZombieTargetType::Player || TargetData.TargetType == EZombieTargetType::NPC)
		{
			// Lower health = higher priority
			float HealthFactor = 1.0f - TargetData.HealthPercentage;
			Score += HealthFactor * HealthWeight * 50.0f;
		}
	}

	// Line of sight bonus
	if (TargetData.bHasLineOfSight)
	{
		Score += LineOfSightBonus;
	}

	// Get dynamic modifier from target
	TScriptInterface<IZombieTargetInterface> TargetInterface = GetTargetInterface(TargetData.Target);
	if (TargetInterface)
	{
		float DynamicMod = TargetInterface->GetDynamicPriorityModifier(OwnerZombie);
		Score *= DynamicMod;
	}

	return Score;
}

AActor* UMultiTargetingAbility::SelectBestTarget()
{
	if (DetectedTargets.Num() == 0)
	{
		return nullptr;
	}

	// Sort by score
	DetectedTargets.Sort([](const FTargetEvaluationData& A, const FTargetEvaluationData& B) {
		return A.FinalScore > B.FinalScore;
	});

	// Get best target
	AActor* NewBestTarget = DetectedTargets[0].Target;
	float NewBestScore = DetectedTargets[0].FinalScore;

	// Check if should switch
	if (BestTarget && BestTarget != NewBestTarget)
	{
		// Find current target's score
		float CurrentScore = 0.0f;
		for (const FTargetEvaluationData& Data : DetectedTargets)
		{
			if (Data.Target == BestTarget)
			{
				CurrentScore = Data.FinalScore;
				break;
			}
		}

		if (!ShouldSwitchTarget(NewBestTarget, NewBestScore, CurrentScore))
		{
			return BestTarget; // Keep current target
		}
	}

	return NewBestTarget;
}

bool UMultiTargetingAbility::ShouldSwitchTarget(AActor* NewTarget, float NewScore, float CurrentScore) const
{
	if (!BestTarget || !NewTarget)
	{
		return true;
	}

	// Always switch to players if enabled
	if (bAlwaysSwitchToPlayers)
	{
		TScriptInterface<IZombieTargetInterface> NewTargetInterface = GetTargetInterface(NewTarget);
		if (NewTargetInterface && NewTargetInterface->GetTargetType() == EZombieTargetType::Player)
		{
			return true;
		}
	}

	// Respect minimum lock time
	if (TimeSinceLastSwitch < MinTargetLockTime)
	{
		return false;
	}

	// New target must be significantly better
	return (NewScore - CurrentScore) >= SwitchThreshold;
}

void UMultiTargetingAbility::SwitchTarget(AActor* NewTarget)
{
	if (NewTarget == BestTarget)
	{
		return;
	}

	// Notify old target
	if (BestTarget)
	{
		TScriptInterface<IZombieTargetInterface> OldInterface = GetTargetInterface(BestTarget);
		if (OldInterface)
		{
			OldInterface->OnUnTargetedByZombie(OwnerZombie);
		}
	}

	PreviousTarget = BestTarget;
	BestTarget = NewTarget;
	TimeSinceLastSwitch = 0.0f;

	// Update zombie base target
	if (ZombieBase)
	{
		ZombieBase->SetTarget(BestTarget);
	}

	// Update ability component target
	CurrentTarget = BestTarget;

	// Notify new target
	if (BestTarget)
	{
		TScriptInterface<IZombieTargetInterface> NewInterface = GetTargetInterface(BestTarget);
		if (NewInterface)
		{
			NewInterface->OnTargetedByZombie(OwnerZombie);
		}
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("MultiTargeting: Switched target to %s"), BestTarget ? *BestTarget->GetName() : TEXT("None"));
	}
}

void UMultiTargetingAbility::ClearCurrentTarget()
{
	SwitchTarget(nullptr);
}

bool UMultiTargetingAbility::IsValidTarget(AActor* Actor)
{
	if (!Actor || !IsValid(Actor))
	{
		return false;
	}

	return Actor->GetClass()->ImplementsInterface(UZombieTargetInterface::StaticClass());
}

TScriptInterface<IZombieTargetInterface> UMultiTargetingAbility::GetTargetInterface(AActor* Actor)
{
	if (!IsValidTarget(Actor))
	{
		return nullptr;
	}

	return TScriptInterface<IZombieTargetInterface>(Actor);
}

bool UMultiTargetingAbility::HasLineOfSight(AActor* Target) const
{
	if (!OwnerZombie || !Target)
	{
		return false;
	}

	FVector Start = OwnerZombie->GetActorLocation() + FVector(0, 0, 50); // Eye height
	FVector End = Target->GetActorLocation();

	FHitResult Hit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerZombie);
	QueryParams.AddIgnoredActor(Target);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		SightTraceChannel,
		QueryParams
	);

	// If no hit or hit the target itself, we have LOS
	return !bHit || Hit.GetActor() == Target;
}

float UMultiTargetingAbility::GetDistanceToTarget(AActor* Target) const
{
	if (!OwnerZombie || !Target)
	{
		return MAX_FLT;
	}

	return FVector::Dist(OwnerZombie->GetActorLocation(), Target->GetActorLocation());
}

// ========================================
// PROTECTED FUNCTIONS
// ========================================

void UMultiTargetingAbility::ProcessDetectedTargets()
{
	AActor* NewBestTarget = SelectBestTarget();

	if (NewBestTarget != BestTarget)
	{
		SwitchTarget(NewBestTarget);
	}
}

void UMultiTargetingAbility::ApplyCoordinationPenalties()
{
	if (!bEnableGroupCoordination)
	{
		return;
	}

	// Apply penalties for overcrowded targets
	for (FTargetEvaluationData& Data : DetectedTargets)
	{
		if (!Data.Target)
		{
			continue;
		}

		TScriptInterface<IZombieTargetInterface> TargetInterface = GetTargetInterface(Data.Target);
		if (!TargetInterface)
		{
			continue;
		}

		// Get number of zombies targeting this
		int32 TargeterCount = TargetInterface->GetZombieTargeterCount();

		// Apply penalty for barricades with too many zombies
		if (Data.TargetType == EZombieTargetType::Barricade)
		{
			if (TargeterCount >= MaxZombiesPerBarricade)
			{
				Data.FinalScore *= 0.1f; // Massive penalty
			}
			else if (bPreferLessCrowdedTargets)
			{
				float CrowdingFactor = (float)TargeterCount / (float)MaxZombiesPerBarricade;
				Data.FinalScore -= CrowdingFactor * OvercrowdingPenalty;
			}
		}
	}
}

bool UMultiTargetingAbility::PassesFilters(AActor* Target, EZombieTargetType TargetType) const
{
	if (!Target)
	{
		return false;
	}

	// Check allowed types
	if (!AllowedTargetTypes.Contains(TargetType))
	{
		return false;
	}

	// Check ignore tags
	for (const FName& Tag : IgnoreTags)
	{
		if (Target->ActorHasTag(Tag))
		{
			return false;
		}
	}

	// Check required tags (if any specified)
	if (RequiredTags.Num() > 0)
	{
		bool bHasRequiredTag = false;
		for (const FName& Tag : RequiredTags)
		{
			if (Target->ActorHasTag(Tag))
			{
				bHasRequiredTag = true;
				break;
			}
		}

		if (!bHasRequiredTag)
		{
			return false;
		}
	}

	return true;
}

float UMultiTargetingAbility::GetTypePriorityScore(EZombieTargetType TargetType) const
{
	const float* Score = TargetTypePriorities.Find(TargetType);
	return Score ? *Score : 50.0f; // Default medium priority
}

float UMultiTargetingAbility::GetPriorityLevelMultiplier(ETargetPriority Priority) const
{
	const float* Multiplier = PriorityLevelMultipliers.Find(Priority);
	return Multiplier ? *Multiplier : 1.0f;
}
