// Copyright Epic Games, Inc. All Rights Reserved.

#include "LeaperAbility.h"
#include "ZombieBase.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "StatusEffectComponent.h"
#include "DrawDebugHelpers.h"

ULeaperAbility::ULeaperAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.05f; // 20 ticks per second for smooth leap

	AbilityName = TEXT("Leaper");
	AbilityDescription = TEXT("Pounces on targets and pins them down");

	// Add required tags
	AbilityTags.Add(TEXT("Leaper"));
	AbilityTags.Add(TEXT("Melee"));
	AbilityTags.Add(TEXT("Mobility"));
}

void ULeaperAbility::BeginPlay()
{
	Super::BeginPlay();
}

void ULeaperAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerZombie || !bIsActive)
	{
		return;
	}

	switch (CurrentLeapState)
	{
	case ELeapState::Idle:
		// Look for targets
		if (CanLeap())
		{
			AActor* Target = FindLeapTarget();
			if (Target)
			{
				LeapTarget = Target;
				CurrentLeapState = ELeapState::Targeting;
			}
		}
		break;

	case ELeapState::Targeting:
		// Auto-execute leap after brief targeting
		ExecuteLeap();
		break;

	case ELeapState::Leaping:
		UpdateLeapMovement(DeltaTime);
		break;

	case ELeapState::Pinning:
		UpdatePinDamage(DeltaTime);

		// Check if pin duration expired
		PinElapsedTime += DeltaTime;
		if (PinElapsedTime >= MaxPinDuration)
		{
			EndPinning(false);
		}
		break;

	case ELeapState::Recovering:
		// Wait for cooldown
		if (GetWorld()->GetTimeSeconds() - LastLeapTime >= LeapCooldown)
		{
			CurrentLeapState = ELeapState::Idle;
		}
		break;
	}
}

// ========================================
// LEAP ABILITY
// ========================================

void ULeaperAbility::ExecuteLeap()
{
	if (!CanLeap() || !LeapTarget)
	{
		return;
	}

	// Calculate trajectory
	FVector LaunchVelocity;
	if (!CalculateLeapTrajectory(LeapTarget, LaunchVelocity))
	{
		OnLeapMissed();
		CurrentLeapState = ELeapState::Recovering;
		LastLeapTime = GetWorld()->GetTimeSeconds();
		return;
	}

	// Set up leap
	LeapStartLocation = OwnerZombie->GetActorLocation();
	LeapTargetLocation = LeapTarget->GetActorLocation();
	LeapElapsedTime = 0.0f;

	// Calculate duration based on distance and speed
	float Distance = FVector::Dist(LeapStartLocation, LeapTargetLocation);
	LeapDuration = Distance / LeapSpeed;

	// Disable character movement
	ACharacter* ZombieCharacter = Cast<ACharacter>(OwnerZombie);
	if (ZombieCharacter && ZombieCharacter->GetCharacterMovement())
	{
		ZombieCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		ZombieCharacter->GetCharacterMovement()->Velocity = LaunchVelocity;
	}

	CurrentLeapState = ELeapState::Leaping;
	LastLeapTime = GetWorld()->GetTimeSeconds();

	OnLeapStarted(LeapTarget);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("LeaperAbility: %s leaping at %s (Distance: %.0f)"),
			*OwnerZombie->GetName(), *LeapTarget->GetName(), Distance);
	}
}

bool ULeaperAbility::CanLeap() const
{
	if (!OwnerZombie || !bIsActive)
	{
		return false;
	}

	// Check cooldown
	if (GetWorld()->GetTimeSeconds() - LastLeapTime < LeapCooldown)
	{
		return false;
	}

	// Check if already leaping or pinning
	if (CurrentLeapState == ELeapState::Leaping || CurrentLeapState == ELeapState::Pinning)
	{
		return false;
	}

	// Check blocking tags
	for (const FName& BlockTag : BlockingTags)
	{
		if (AbilityTags.Contains(BlockTag))
		{
			return false;
		}
	}

	return true;
}

bool ULeaperAbility::IsLeaping() const
{
	return CurrentLeapState == ELeapState::Leaping;
}

bool ULeaperAbility::IsPinning() const
{
	return CurrentLeapState == ELeapState::Pinning;
}

void ULeaperAbility::BreakFree(float Force)
{
	if (CurrentLeapState != ELeapState::Pinning)
	{
		return;
	}

	AccumulatedBreakForce += Force;

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("LeaperAbility: Break force applied %.0f / %.0f"),
			AccumulatedBreakForce, BreakFreeThreshold);
	}

	if (AccumulatedBreakForce >= BreakFreeThreshold)
	{
		EndPinning(true);
	}
}

// ========================================
// PROTECTED FUNCTIONS
// ========================================

AActor* ULeaperAbility::FindLeapTarget()
{
	if (!OwnerZombie)
	{
		return nullptr;
	}

	TArray<FHitResult> HitResults;
	FCollisionShape SphereShape = FCollisionShape::MakeSphere(DetectionRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(OwnerZombie);

	GetWorld()->SweepMultiByChannel(
		HitResults,
		OwnerZombie->GetActorLocation(),
		OwnerZombie->GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		SphereShape,
		QueryParams
	);

	AActor* BestTarget = nullptr;
	float BestScore = -1.0f;

	for (const FHitResult& Hit : HitResults)
	{
		AActor* PotentialTarget = Hit.GetActor();
		if (!PotentialTarget)
		{
			continue;
		}

		// Check if it's a valid target (player, NPC, etc.)
		ACharacter* Character = Cast<ACharacter>(PotentialTarget);
		if (!Character)
		{
			continue;
		}

		float Distance = FVector::Dist(OwnerZombie->GetActorLocation(), PotentialTarget->GetActorLocation());

		// Must be within leap range
		if (Distance < MinLeapRange || Distance > MaxLeapRange)
		{
			continue;
		}

		// Check line of sight
		FHitResult LOSHit;
		FVector Start = OwnerZombie->GetActorLocation();
		FVector End = PotentialTarget->GetActorLocation();

		if (GetWorld()->LineTraceSingleByChannel(LOSHit, Start, End, ECC_Visibility, QueryParams))
		{
			if (LOSHit.GetActor() != PotentialTarget)
			{
				continue; // Something blocking LOS
			}
		}

		// Score based on distance (prefer closer targets)
		float Score = 1.0f - (Distance / MaxLeapRange);

		// Prefer targets in front
		FVector ToTarget = (PotentialTarget->GetActorLocation() - OwnerZombie->GetActorLocation()).GetSafeNormal();
		FVector Forward = OwnerZombie->GetActorForwardVector();
		float DotProduct = FVector::DotProduct(Forward, ToTarget);
		Score *= (DotProduct + 1.0f) * 0.5f; // Normalize to 0-1

		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = PotentialTarget;
		}
	}

	return BestTarget;
}

bool ULeaperAbility::CalculateLeapTrajectory(AActor* Target, FVector& OutVelocity)
{
	if (!Target || !OwnerZombie)
	{
		return false;
	}

	FVector Start = OwnerZombie->GetActorLocation();
	FVector End = Target->GetActorLocation();
	float Distance = FVector::Dist2D(Start, End);
	float HeightDifference = End.Z - Start.Z;

	// Calculate launch angle in radians
	float AngleRad = FMath::DegreesToRadians(LeapAngle);

	// Calculate initial velocity magnitude needed
	float Gravity = FMath::Abs(GetWorld()->GetGravityZ());
	float VelocityMagnitude = FMath::Sqrt((Distance * Gravity) / FMath::Sin(2.0f * AngleRad));

	// Calculate velocity components
	FVector HorizontalDirection = (End - Start).GetSafeNormal2D();
	float HorizontalVelocity = VelocityMagnitude * FMath::Cos(AngleRad);
	float VerticalVelocity = VelocityMagnitude * FMath::Sin(AngleRad);

	// Apply arc height multiplier
	VerticalVelocity *= LeapArcHeight;

	OutVelocity = HorizontalDirection * HorizontalVelocity;
	OutVelocity.Z = VerticalVelocity;

	return true;
}

void ULeaperAbility::UpdateLeapMovement(float DeltaTime)
{
	if (!OwnerZombie || !LeapTarget)
	{
		return;
	}

	LeapElapsedTime += DeltaTime;

	// Check for collision with target
	if (CheckLeapCollision())
	{
		ApplyLeapDamage(LeapTarget);
		StartPinning(LeapTarget);
		return;
	}

	// Check if leap duration exceeded (missed)
	if (LeapElapsedTime >= LeapDuration * 1.5f)
	{
		OnLeapMissed();

		// Restore normal movement
		ACharacter* ZombieCharacter = Cast<ACharacter>(OwnerZombie);
		if (ZombieCharacter && ZombieCharacter->GetCharacterMovement())
		{
			ZombieCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		CurrentLeapState = ELeapState::Recovering;
		LeapTarget = nullptr;
	}

	// Debug visualization
	if (bShowDebug)
	{
		DrawDebugLine(GetWorld(), OwnerZombie->GetActorLocation(),
			LeapTargetLocation, FColor::Red, false, 0.1f, 0, 2.0f);
	}
}

bool ULeaperAbility::CheckLeapCollision()
{
	if (!OwnerZombie || !LeapTarget)
	{
		return false;
	}

	float Distance = FVector::Dist(OwnerZombie->GetActorLocation(), LeapTarget->GetActorLocation());

	// Check if close enough to land
	return Distance <= 150.0f; // Collision threshold
}

void ULeaperAbility::StartPinning(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	PinnedTarget = Target;
	PinElapsedTime = 0.0f;
	AccumulatedBreakForce = 0.0f;
	CurrentLeapState = ELeapState::Pinning;

	// Restore walking movement
	ACharacter* ZombieCharacter = Cast<ACharacter>(OwnerZombie);
	if (ZombieCharacter && ZombieCharacter->GetCharacterMovement())
	{
		ZombieCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		ZombieCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}

	// Apply stun to target
	UStatusEffectComponent* TargetEffects = Target->FindComponentByClass<UStatusEffectComponent>();
	if (TargetEffects)
	{
		TargetEffects->ApplyStatusEffect(EStatusEffectType::Stun, 1.0f, StunDuration, OwnerZombie);
	}

	// Disable target movement
	ACharacter* TargetCharacter = Cast<ACharacter>(Target);
	if (TargetCharacter && TargetCharacter->GetCharacterMovement())
	{
		TargetCharacter->GetCharacterMovement()->DisableMovement();
	}

	OnPinStarted(Target);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("LeaperAbility: %s pinning %s"),
			*OwnerZombie->GetName(), *Target->GetName());
	}
}

void ULeaperAbility::EndPinning(bool bBrokenFree)
{
	if (!PinnedTarget)
	{
		return;
	}

	// Re-enable target movement
	ACharacter* TargetCharacter = Cast<ACharacter>(PinnedTarget);
	if (TargetCharacter && TargetCharacter->GetCharacterMovement())
	{
		TargetCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}

	OnPinEnded(PinnedTarget, bBrokenFree);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("LeaperAbility: Pin ended (Broken free: %s)"),
			bBrokenFree ? TEXT("Yes") : TEXT("No"));
	}

	PinnedTarget = nullptr;
	LeapTarget = nullptr;
	CurrentLeapState = ELeapState::Recovering;
	LastLeapTime = GetWorld()->GetTimeSeconds();
}

void ULeaperAbility::UpdatePinDamage(float DeltaTime)
{
	if (!PinnedTarget || !OwnerZombie)
	{
		return;
	}

	// Apply damage over time
	float DamageThisTick = PinDamagePerSecond * DeltaTime;

	UGameplayStatics::ApplyDamage(
		PinnedTarget,
		DamageThisTick,
		OwnerZombie->GetInstigatorController(),
		OwnerZombie,
		UDamageType::StaticClass()
	);

	// Keep leaper attached to target
	FVector TargetLocation = PinnedTarget->GetActorLocation();
	FVector Offset = FVector(0.0f, 0.0f, 50.0f); // Slightly above target
	OwnerZombie->SetActorLocation(TargetLocation + Offset);
}

void ULeaperAbility::ApplyLeapDamage(AActor* Target)
{
	if (!Target || !OwnerZombie)
	{
		return;
	}

	UGameplayStatics::ApplyDamage(
		Target,
		ImpactDamage,
		OwnerZombie->GetInstigatorController(),
		OwnerZombie,
		UDamageType::StaticClass()
	);

	OnLeapImpact(Target, ImpactDamage);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("LeaperAbility: Impact damage %.0f dealt to %s"),
			ImpactDamage, *Target->GetName());
	}
}

// ========================================
// EVENTS
// ========================================

void ULeaperAbility::OnLeapStarted_Implementation(AActor* Target)
{
	// Override in Blueprint
}

void ULeaperAbility::OnLeapImpact_Implementation(AActor* Target, float Damage)
{
	// Override in Blueprint
}

void ULeaperAbility::OnPinStarted_Implementation(AActor* Target)
{
	// Override in Blueprint
}

void ULeaperAbility::OnPinEnded_Implementation(AActor* Target, bool bBrokenFree)
{
	// Override in Blueprint
}

void ULeaperAbility::OnLeapMissed_Implementation()
{
	// Override in Blueprint
}
