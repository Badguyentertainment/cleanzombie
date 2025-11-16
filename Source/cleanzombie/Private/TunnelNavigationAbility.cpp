// Copyright Epic Games, Inc. All Rights Reserved.

#include "TunnelNavigationAbility.h"
#include "TunnelVolume.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

UTunnelNavigationAbility::UTunnelNavigationAbility()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;

	AbilityName = FText::FromString(TEXT("Tunnel Navigation"));
	AbilityDescription = FText::FromString(TEXT("Navigate through tunnel and vent systems"));
	AbilityPriority = 80; // High priority - overrides normal movement
	bCanRunConcurrently = false;

	// Add blocking tags
	BlockingTags.Add(TEXT("Climbing"));
	BlockingTags.Add(TEXT("Stunned"));

	// Add ability tags
	AbilityTags.Add(TEXT("Tunneling"));
	AbilityTags.Add(TEXT("Moving"));

	// Set to replicate
	SetIsReplicatedByDefault(true);
}

void UTunnelNavigationAbility::BeginPlay()
{
	Super::BeginPlay();

	// Cache components
	if (OwnerZombie)
	{
		CachedCapsule = OwnerZombie->FindComponentByClass<UCapsuleComponent>();
		CachedMovement = OwnerZombie->FindComponentByClass<UCharacterMovementComponent>();

		if (CachedCapsule)
		{
			OriginalCapsuleRadius = CachedCapsule->GetUnscaledCapsuleRadius();
			OriginalCapsuleHalfHeight = CachedCapsule->GetUnscaledCapsuleHalfHeight();
		}

		USkeletalMeshComponent* Mesh = OwnerZombie->GetMesh();
		if (Mesh)
		{
			OriginalMeshScale = Mesh->GetRelativeScale3D();
		}
	}
}

void UTunnelNavigationAbility::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsActive)
	{
		return;
	}

	UpdateAbility(DeltaTime);
}

void UTunnelNavigationAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTunnelNavigationAbility, TunnelData);
}

// ========================================
// ABILITY OVERRIDES
// ========================================

void UTunnelNavigationAbility::InitializeAbility_Implementation()
{
	Super::InitializeAbility_Implementation();
}

bool UTunnelNavigationAbility::ActivateAbility_Implementation()
{
	if (!Super::ActivateAbility_Implementation())
	{
		return false;
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Activated for %s"), *GetOwner()->GetName());
	}

	return true;
}

void UTunnelNavigationAbility::DeactivateAbility_Implementation()
{
	// Clean up tunnel state
	if (IsInTunnel())
	{
		ExitTunnel();
	}

	Super::DeactivateAbility_Implementation();

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Deactivated for %s"), *GetOwner()->GetName());
	}
}

void UTunnelNavigationAbility::UpdateAbility_Implementation(float DeltaTime)
{
	Super::UpdateAbility_Implementation(DeltaTime);

	if (!bIsActive || !IsInTunnel())
	{
		return;
	}

	// Update based on traversal state
	switch (TunnelData.TraversalState)
	{
	case ETunnelTraversalState::Entering:
		ProcessTunnelEntry(DeltaTime);
		break;

	case ETunnelTraversalState::Traversing:
		ProcessTunnelTraversal(DeltaTime);
		break;

	case ETunnelTraversalState::Exiting:
		ProcessTunnelExit(DeltaTime);
		break;

	default:
		break;
	}

	TunnelData.TraversalTime += DeltaTime;
}

bool UTunnelNavigationAbility::CanActivate_Implementation() const
{
	if (!Super::CanActivate_Implementation())
	{
		return false;
	}

	// Can activate if we have a valid tunnel nearby
	if (bAutoDetectTunnels)
	{
		int32 DummyIndex;
		ATunnelVolume* NearbyTunnel = FindNearestTunnel(TunnelDetectionRadius, DummyIndex);
		return NearbyTunnel != nullptr;
	}

	return true;
}

// ========================================
// TUNNEL NAVIGATION
// ========================================

bool UTunnelNavigationAbility::EnterTunnel(ATunnelVolume* Tunnel, int32 EntryPointIndex, const FVector& TargetLocation)
{
	if (!Tunnel || !OwnerZombie)
	{
		return false;
	}

	if (!Tunnel->CanActorEnter(OwnerZombie, EntryPointIndex))
	{
		if (bShowDebug)
		{
			UE_LOG(LogTemp, Warning, TEXT("TunnelNavigationAbility: Cannot enter tunnel - permission denied"));
		}
		return false;
	}

	// Activate ability if not active
	if (!bIsActive)
	{
		ActivateAbility();
	}

	// Setup tunnel data
	TunnelData.CurrentTunnel = Tunnel;
	TunnelData.EntryPointIndex = EntryPointIndex;
	TunnelData.ExitPointIndex = Tunnel->FindBestExitPoint(TargetLocation);
	TunnelData.CurrentDistance = 0.0f;
	TunnelData.TotalDistance = Tunnel->GetTunnelLength(EntryPointIndex, TunnelData.ExitPointIndex);
	TunnelData.TraversalState = ETunnelTraversalState::Entering;
	TunnelData.TraversalTime = 0.0f;
	TunnelData.ExitTargetLocation = TargetLocation;

	// Get entry point spline distance
	FTunnelEntryPoint EntryPoint = Tunnel->GetEntryPoint(EntryPointIndex);
	TunnelData.CurrentDistance = Tunnel->TunnelSpline->GetDistanceAlongSplineAtSplinePoint(EntryPoint.SplinePointIndex);

	// Modify zombie for tunnel
	ModifyForTunnel(Tunnel);

	OnTunnelEntered(Tunnel, EntryPointIndex);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Entered tunnel at point %d, target exit %d (Distance: %.1f)"),
			EntryPointIndex, TunnelData.ExitPointIndex, TunnelData.TotalDistance);
	}

	return true;
}

void UTunnelNavigationAbility::ExitTunnel()
{
	if (!IsInTunnel())
	{
		return;
	}

	ATunnelVolume* Tunnel = TunnelData.CurrentTunnel;
	int32 ExitIndex = TunnelData.ExitPointIndex;

	// Restore zombie
	RestoreFromTunnel();

	// Notify tunnel
	if (Tunnel)
	{
		Tunnel->OnActorExitedTunnel(OwnerZombie, ExitIndex);
	}

	OnTunnelExited(Tunnel, ExitIndex);

	// Clear tunnel data
	TunnelData = FTunnelNavigationData();

	// Deactivate ability
	DeactivateAbility();

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Exited tunnel at point %d"), ExitIndex);
	}
}

void UTunnelNavigationAbility::AbortTunnelTraversal()
{
	if (!IsInTunnel())
	{
		return;
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Warning, TEXT("TunnelNavigationAbility: Aborting tunnel traversal"));
	}

	OnTunnelAborted();
	ExitTunnel();
}

bool UTunnelNavigationAbility::IsInTunnel() const
{
	return TunnelData.CurrentTunnel != nullptr && TunnelData.TraversalState != ETunnelTraversalState::None;
}

bool UTunnelNavigationAbility::IsTraversingTunnel() const
{
	return TunnelData.TraversalState == ETunnelTraversalState::Traversing;
}

float UTunnelNavigationAbility::GetTraversalProgress() const
{
	if (!IsInTunnel() || TunnelData.TotalDistance <= 0.0f)
	{
		return 0.0f;
	}

	FTunnelEntryPoint EntryPoint = TunnelData.CurrentTunnel->GetEntryPoint(TunnelData.EntryPointIndex);
	float EntryDistance = TunnelData.CurrentTunnel->TunnelSpline->GetDistanceAlongSplineAtSplinePoint(EntryPoint.SplinePointIndex);

	float TraveledDistance = TunnelData.CurrentDistance - EntryDistance;
	return FMath::Clamp(TraveledDistance / TunnelData.TotalDistance, 0.0f, 1.0f);
}

ATunnelVolume* UTunnelNavigationAbility::FindNearestTunnel(float MaxDistance, int32& OutEntryPointIndex) const
{
	if (!OwnerZombie)
	{
		return nullptr;
	}

	FVector ZombieLocation = OwnerZombie->GetActorLocation();
	ATunnelVolume* NearestTunnel = nullptr;
	float NearestDistSq = MaxDistance * MaxDistance;
	OutEntryPointIndex = -1;

	TArray<ATunnelVolume*> AllTunnels = GetAllTunnelsInWorld();

	for (ATunnelVolume* Tunnel : AllTunnels)
	{
		if (!Tunnel || !CanUseTunnel(Tunnel))
		{
			continue;
		}

		int32 EntryIndex = Tunnel->GetClosestEntryPoint(ZombieLocation, true);
		if (EntryIndex < 0)
		{
			continue;
		}

		FTunnelEntryPoint EntryPoint = Tunnel->GetEntryPoint(EntryIndex);
		float DistSq = FVector::DistSquared(ZombieLocation, EntryPoint.Location);

		if (DistSq < NearestDistSq)
		{
			NearestDistSq = DistSq;
			NearestTunnel = Tunnel;
			OutEntryPointIndex = EntryIndex;
		}
	}

	return NearestTunnel;
}

bool UTunnelNavigationAbility::CanUseTunnel(ATunnelVolume* Tunnel) const
{
	if (!Tunnel || !OwnerZombie)
	{
		return false;
	}

	return Tunnel->bZombiesCanUse;
}

ATunnelVolume* UTunnelNavigationAbility::FindBestTunnelToTarget(const FVector& TargetLocation, float MaxSearchRadius,
	int32& OutEntryIndex, int32& OutExitIndex) const
{
	if (!OwnerZombie)
	{
		return nullptr;
	}

	FVector ZombieLocation = OwnerZombie->GetActorLocation();
	ATunnelVolume* BestTunnel = nullptr;
	float BestScore = FLT_MAX;
	OutEntryIndex = -1;
	OutExitIndex = -1;

	TArray<ATunnelVolume*> AllTunnels = GetAllTunnelsInWorld();

	for (ATunnelVolume* Tunnel : AllTunnels)
	{
		if (!Tunnel || !CanUseTunnel(Tunnel))
		{
			continue;
		}

		// Find best entry point (closest to zombie)
		int32 EntryIndex = Tunnel->GetClosestEntryPoint(ZombieLocation, true);
		if (EntryIndex < 0)
		{
			continue;
		}

		FTunnelEntryPoint EntryPoint = Tunnel->GetEntryPoint(EntryIndex);
		float DistToEntry = FVector::Dist(ZombieLocation, EntryPoint.Location);

		if (DistToEntry > MaxSearchRadius)
		{
			continue;
		}

		// Find best exit point (closest to target)
		int32 ExitIndex = Tunnel->FindBestExitPoint(TargetLocation);
		if (ExitIndex < 0)
		{
			continue;
		}

		FTunnelEntryPoint ExitPoint = Tunnel->GetEntryPoint(ExitIndex);
		float DistFromExit = FVector::Dist(ExitPoint.Location, TargetLocation);

		// Calculate total path distance
		float TunnelLength = Tunnel->GetTunnelLength(EntryIndex, ExitIndex);
		float TotalDistance = DistToEntry + TunnelLength + DistFromExit;

		// Calculate direct distance for comparison
		float DirectDistance = FVector::Dist(ZombieLocation, TargetLocation);

		// Score based on whether tunnel is a shortcut
		float Score = TotalDistance;

		// Prefer tunnels if configured
		if (bPreferTunnels)
		{
			Score *= 0.8f; // 20% bonus for using tunnels
		}

		// Penalize if tunnel is a significant detour
		float DetourAmount = TotalDistance - DirectDistance;
		if (DetourAmount > MaxTunnelDetourDistance)
		{
			continue; // Skip this tunnel, too much detour
		}

		// Apply tunnel priority
		Score /= FMath::Max(Tunnel->PathfindingPriority, 0.1f);

		if (Score < BestScore)
		{
			BestScore = Score;
			BestTunnel = Tunnel;
			OutEntryIndex = EntryIndex;
			OutExitIndex = ExitIndex;
		}
	}

	return BestTunnel;
}

// ========================================
// EVENTS
// ========================================

void UTunnelNavigationAbility::OnTunnelEntered_Implementation(ATunnelVolume* Tunnel, int32 EntryPointIndex)
{
	// Override in Blueprint
}

void UTunnelNavigationAbility::OnTunnelExited_Implementation(ATunnelVolume* Tunnel, int32 ExitPointIndex)
{
	// Override in Blueprint
}

void UTunnelNavigationAbility::OnTunnelTraversing_Implementation(float Progress)
{
	// Override in Blueprint
}

void UTunnelNavigationAbility::OnTunnelAborted_Implementation()
{
	// Override in Blueprint
}

// ========================================
// PROTECTED FUNCTIONS
// ========================================

void UTunnelNavigationAbility::ProcessTunnelEntry(float DeltaTime)
{
	TunnelData.TraversalTime += DeltaTime;

	if (TunnelData.TraversalTime >= EnterDuration)
	{
		// Transition to traversing
		TunnelData.TraversalState = ETunnelTraversalState::Traversing;
		TunnelData.TraversalTime = 0.0f;

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Entering complete, now traversing"));
		}
	}
}

void UTunnelNavigationAbility::ProcessTunnelTraversal(float DeltaTime)
{
	if (!TunnelData.CurrentTunnel || !OwnerZombie)
	{
		return;
	}

	// Move along spline
	UpdateSplineMovement(DeltaTime);

	// Check if reached exit
	FTunnelEntryPoint ExitPoint = TunnelData.CurrentTunnel->GetEntryPoint(TunnelData.ExitPointIndex);
	float ExitDistance = TunnelData.CurrentTunnel->TunnelSpline->GetDistanceAlongSplineAtSplinePoint(ExitPoint.SplinePointIndex);

	float DistanceToExit = FMath::Abs(TunnelData.CurrentDistance - ExitDistance);

	if (DistanceToExit < 50.0f) // Within 50 units of exit
	{
		// Transition to exiting
		TunnelData.TraversalState = ETunnelTraversalState::Exiting;
		TunnelData.TraversalTime = 0.0f;

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Reached exit, now exiting"));
		}
	}

	// Notify traversal progress
	float Progress = GetTraversalProgress();
	OnTunnelTraversing(Progress);

	if (TunnelData.CurrentTunnel)
	{
		TunnelData.CurrentTunnel->OnActorTraversingTunnel(OwnerZombie, TunnelData.CurrentDistance);
	}
}

void UTunnelNavigationAbility::ProcessTunnelExit(float DeltaTime)
{
	TunnelData.TraversalTime += DeltaTime;

	if (TunnelData.TraversalTime >= ExitDuration)
	{
		// Complete exit
		ExitTunnel();
	}
}

void UTunnelNavigationAbility::ModifyForTunnel(ATunnelVolume* Tunnel)
{
	if (!Tunnel || !OwnerZombie)
	{
		return;
	}

	// Modify capsule collision
	if (CachedCapsule)
	{
		CachedCapsule->SetCapsuleSize(Tunnel->TunnelCapsuleRadius, Tunnel->TunnelCapsuleHalfHeight);

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Modified capsule - Radius: %.1f, HalfHeight: %.1f"),
				Tunnel->TunnelCapsuleRadius, Tunnel->TunnelCapsuleHalfHeight);
		}
	}

	// Scale mesh
	USkeletalMeshComponent* Mesh = OwnerZombie->GetMesh();
	if (Mesh)
	{
		FVector NewScale = OriginalMeshScale * Tunnel->ZombieScaleInTunnel;
		Mesh->SetRelativeScale3D(NewScale);

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Modified mesh scale to %.2f"), Tunnel->ZombieScaleInTunnel);
		}
	}

	// Disable gravity and flying movement
	if (CachedMovement)
	{
		CachedMovement->GravityScale = 0.0f;
		CachedMovement->SetMovementMode(MOVE_Flying);
	}
}

void UTunnelNavigationAbility::RestoreFromTunnel()
{
	if (!OwnerZombie)
	{
		return;
	}

	// Restore capsule
	if (CachedCapsule && bRestoreCollisionOnExit)
	{
		CachedCapsule->SetCapsuleSize(OriginalCapsuleRadius, OriginalCapsuleHalfHeight);

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Restored capsule size"));
		}
	}

	// Restore mesh scale
	if (bRestoreMeshScaleOnExit)
	{
		USkeletalMeshComponent* Mesh = OwnerZombie->GetMesh();
		if (Mesh)
		{
			Mesh->SetRelativeScale3D(OriginalMeshScale);

			if (bShowDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("TunnelNavigationAbility: Restored mesh scale"));
			}
		}
	}

	// Restore movement
	if (CachedMovement)
	{
		CachedMovement->GravityScale = 1.0f;
		CachedMovement->SetMovementMode(MOVE_Walking);
	}
}

void UTunnelNavigationAbility::UpdateSplineMovement(float DeltaTime)
{
	if (!TunnelData.CurrentTunnel || !OwnerZombie)
	{
		return;
	}

	// Calculate movement distance
	float MovementSpeed = TunnelMovementSpeed * TunnelData.CurrentTunnel->TunnelSpeedMultiplier;
	float MovementDistance = MovementSpeed * DeltaTime;

	// Determine direction (towards exit)
	FTunnelEntryPoint EntryPoint = TunnelData.CurrentTunnel->GetEntryPoint(TunnelData.EntryPointIndex);
	FTunnelEntryPoint ExitPoint = TunnelData.CurrentTunnel->GetEntryPoint(TunnelData.ExitPointIndex);

	float EntryDist = TunnelData.CurrentTunnel->TunnelSpline->GetDistanceAlongSplineAtSplinePoint(EntryPoint.SplinePointIndex);
	float ExitDist = TunnelData.CurrentTunnel->TunnelSpline->GetDistanceAlongSplineAtSplinePoint(ExitPoint.SplinePointIndex);

	float Direction = ExitDist > EntryDist ? 1.0f : -1.0f;

	// Update distance
	TunnelData.CurrentDistance += MovementDistance * Direction;
	TunnelData.CurrentDistance = FMath::Clamp(TunnelData.CurrentDistance, 0.0f, TunnelData.CurrentTunnel->GetTotalSplineLength());

	// Get transform at current distance
	FTransform SplineTransform = TunnelData.CurrentTunnel->GetTransformAtDistance(TunnelData.CurrentDistance);

	// Set zombie transform
	OwnerZombie->SetActorLocation(SplineTransform.GetLocation());
	OwnerZombie->SetActorRotation(SplineTransform.GetRotation());
}

TArray<ATunnelVolume*> UTunnelNavigationAbility::GetAllTunnelsInWorld() const
{
	TArray<ATunnelVolume*> Tunnels;

	if (!GetWorld())
	{
		return Tunnels;
	}

	for (TActorIterator<ATunnelVolume> It(GetWorld()); It; ++It)
	{
		Tunnels.Add(*It);
	}

	return Tunnels;
}
