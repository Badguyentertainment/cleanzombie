// Copyright Epic Games, Inc. All Rights Reserved.

#include "TunnelVolume.h"
#include "Components/SplineComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ATunnelVolume::ATunnelVolume()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root
	TunnelRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TunnelRoot"));
	RootComponent = TunnelRoot;

	// Create spline
	TunnelSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TunnelSpline"));
	TunnelSpline->SetupAttachment(RootComponent);
	TunnelSpline->SetClosedLoop(false);

	// Default spline setup
	TunnelSpline->ClearSplinePoints(true);
	TunnelSpline->AddSplinePoint(FVector(0, 0, 0), ESplineCoordinateSpace::Local, true);
	TunnelSpline->AddSplinePoint(FVector(500, 0, 0), ESplineCoordinateSpace::Local, true);
}

void ATunnelVolume::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoCreateTriggers)
	{
		CreateTriggerVolumes();
	}
}

void ATunnelVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Update entry point locations from spline
	for (FTunnelEntryPoint& Entry : EntryPoints)
	{
		if (Entry.SplinePointIndex >= 0 && Entry.SplinePointIndex < TunnelSpline->GetNumberOfSplinePoints())
		{
			Entry.Location = TunnelSpline->GetLocationAtSplinePoint(Entry.SplinePointIndex, ESplineCoordinateSpace::World);
			Entry.EntryRotation = TunnelSpline->GetRotationAtSplinePoint(Entry.SplinePointIndex, ESplineCoordinateSpace::World);
		}
	}
}

void ATunnelVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug visualization
	if (bShowDebug)
	{
		if (bDrawSpline && TunnelSpline)
		{
			TunnelSpline->DrawDebug(GetWorld(), FColor::Cyan, 0.0f, 2.0f);
		}

		if (bDrawEntryPoints)
		{
			for (int32 i = 0; i < EntryPoints.Num(); i++)
			{
				const FTunnelEntryPoint& Entry = EntryPoints[i];
				FColor Color = Entry.bIsEntrance ? FColor::Green : FColor::Red;
				if (Entry.bIsEntrance && Entry.bIsExit)
				{
					Color = FColor::Yellow;
				}

				DrawDebugSphere(GetWorld(), Entry.Location, 50.0f, 12, Color, false, 0.0f, 0, 2.0f);
				DrawDebugDirectionalArrow(GetWorld(), Entry.Location, Entry.Location + Entry.EntryRotation.Vector() * 100.0f,
					50.0f, Color, false, 0.0f, 0, 2.0f);
			}
		}

		if (bDrawTriggers)
		{
			for (const auto& Pair : TriggerToEntryPointMap)
			{
				if (Pair.Key)
				{
					FVector BoxLocation = Pair.Key->GetComponentLocation();
					FVector BoxExtent = Pair.Key->GetScaledBoxExtent();
					DrawDebugBox(GetWorld(), BoxLocation, BoxExtent, Pair.Key->GetComponentQuat(), FColor::Blue, false, 0.0f, 0, 2.0f);
				}
			}
		}
	}
}

// ========================================
// TUNNEL FUNCTIONS
// ========================================

bool ATunnelVolume::CanActorEnter(AActor* Actor, int32 EntryPointIndex) const
{
	if (!Actor || !bZombiesCanUse)
	{
		return false;
	}

	if (EntryPointIndex < 0 || EntryPointIndex >= EntryPoints.Num())
	{
		return false;
	}

	const FTunnelEntryPoint& Entry = EntryPoints[EntryPointIndex];
	if (!Entry.bIsEntrance)
	{
		return false;
	}

	// Check if actor is a player
	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn && Pawn->IsPlayerControlled())
	{
		return bPlayersCanUse;
	}

	return true;
}

int32 ATunnelVolume::GetClosestEntryPoint(const FVector& Location, bool bEntrancesOnly) const
{
	int32 ClosestIndex = -1;
	float ClosestDistSq = FLT_MAX;

	for (int32 i = 0; i < EntryPoints.Num(); i++)
	{
		const FTunnelEntryPoint& Entry = EntryPoints[i];

		if (bEntrancesOnly && !Entry.bIsEntrance)
		{
			continue;
		}

		float DistSq = FVector::DistSquared(Location, Entry.Location);
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestIndex = i;
		}
	}

	return ClosestIndex;
}

FTunnelEntryPoint ATunnelVolume::GetEntryPoint(int32 Index) const
{
	if (Index >= 0 && Index < EntryPoints.Num())
	{
		return EntryPoints[Index];
	}

	return FTunnelEntryPoint();
}

TArray<FTunnelEntryPoint> ATunnelVolume::GetEntrancePoints() const
{
	TArray<FTunnelEntryPoint> Entrances;

	for (const FTunnelEntryPoint& Entry : EntryPoints)
	{
		if (Entry.bIsEntrance)
		{
			Entrances.Add(Entry);
		}
	}

	return Entrances;
}

TArray<FTunnelEntryPoint> ATunnelVolume::GetExitPoints() const
{
	TArray<FTunnelEntryPoint> Exits;

	for (const FTunnelEntryPoint& Entry : EntryPoints)
	{
		if (Entry.bIsExit)
		{
			Exits.Add(Entry);
		}
	}

	return Exits;
}

float ATunnelVolume::GetTunnelLength(int32 EntryIndex, int32 ExitIndex) const
{
	if (!TunnelSpline)
	{
		return 0.0f;
	}

	if (EntryIndex < 0 || EntryIndex >= EntryPoints.Num() ||
		ExitIndex < 0 || ExitIndex >= EntryPoints.Num())
	{
		return 0.0f;
	}

	const FTunnelEntryPoint& Entry = EntryPoints[EntryIndex];
	const FTunnelEntryPoint& Exit = EntryPoints[ExitIndex];

	float EntryDist = TunnelSpline->GetDistanceAlongSplineAtSplinePoint(Entry.SplinePointIndex);
	float ExitDist = TunnelSpline->GetDistanceAlongSplineAtSplinePoint(Exit.SplinePointIndex);

	return FMath::Abs(ExitDist - EntryDist);
}

FVector ATunnelVolume::GetLocationAtDistance(float Distance) const
{
	if (!TunnelSpline)
	{
		return FVector::ZeroVector;
	}

	return TunnelSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FRotator ATunnelVolume::GetRotationAtDistance(float Distance) const
{
	if (!TunnelSpline)
	{
		return FRotator::ZeroRotator;
	}

	return TunnelSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FTransform ATunnelVolume::GetTransformAtDistance(float Distance) const
{
	if (!TunnelSpline)
	{
		return FTransform::Identity;
	}

	return TunnelSpline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FVector ATunnelVolume::GetDirectionAtDistance(float Distance) const
{
	if (!TunnelSpline)
	{
		return FVector::ForwardVector;
	}

	return TunnelSpline->GetDirectionAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

float ATunnelVolume::GetTotalSplineLength() const
{
	if (!TunnelSpline)
	{
		return 0.0f;
	}

	return TunnelSpline->GetSplineLength();
}

int32 ATunnelVolume::FindBestExitPoint(const FVector& TargetLocation) const
{
	int32 BestExitIndex = -1;
	float BestScore = FLT_MAX;

	for (int32 i = 0; i < EntryPoints.Num(); i++)
	{
		const FTunnelEntryPoint& Entry = EntryPoints[i];

		if (!Entry.bIsExit)
		{
			continue;
		}

		// Score based on distance to target
		float DistToTarget = FVector::Dist(Entry.Location, TargetLocation);

		if (DistToTarget < BestScore)
		{
			BestScore = DistToTarget;
			BestExitIndex = i;
		}
	}

	return BestExitIndex;
}

bool ATunnelVolume::IsLocationInTunnel(const FVector& Location, float Tolerance) const
{
	if (!TunnelSpline)
	{
		return false;
	}

	// Find closest point on spline
	float InputKey = TunnelSpline->FindInputKeyClosestToWorldLocation(Location);
	FVector ClosestPoint = TunnelSpline->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);

	float Distance = FVector::Dist(Location, ClosestPoint);
	return Distance <= Tolerance;
}

// ========================================
// EDITOR UTILITIES
// ========================================

void ATunnelVolume::RebuildTriggerVolumes()
{
	CleanupTriggerVolumes();
	CreateTriggerVolumes();

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelVolume: Rebuilt %d trigger volumes"), EntryPoints.Num());
	}
}

void ATunnelVolume::AddEntryPointAtSplinePoint(int32 SplinePointIndex)
{
	if (!TunnelSpline || SplinePointIndex < 0 || SplinePointIndex >= TunnelSpline->GetNumberOfSplinePoints())
	{
		return;
	}

	FTunnelEntryPoint NewEntry;
	NewEntry.SplinePointIndex = SplinePointIndex;
	NewEntry.Location = TunnelSpline->GetLocationAtSplinePoint(SplinePointIndex, ESplineCoordinateSpace::World);
	NewEntry.EntryRotation = TunnelSpline->GetRotationAtSplinePoint(SplinePointIndex, ESplineCoordinateSpace::World);
	NewEntry.EntryTag = FName(*FString::Printf(TEXT("Entry_%d"), EntryPoints.Num()));

	EntryPoints.Add(NewEntry);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelVolume: Added entry point at spline point %d"), SplinePointIndex);
	}
}

void ATunnelVolume::AutoGenerateEntryPoints()
{
	if (!TunnelSpline)
	{
		return;
	}

	ClearEntryPoints();

	int32 NumPoints = TunnelSpline->GetNumberOfSplinePoints();

	// Add entrance at start
	if (NumPoints > 0)
	{
		FTunnelEntryPoint StartEntry;
		StartEntry.SplinePointIndex = 0;
		StartEntry.Location = TunnelSpline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
		StartEntry.EntryRotation = TunnelSpline->GetRotationAtSplinePoint(0, ESplineCoordinateSpace::World);
		StartEntry.bIsEntrance = true;
		StartEntry.bIsExit = true;
		StartEntry.EntryTag = TEXT("Entry_Start");
		EntryPoints.Add(StartEntry);
	}

	// Add exit at end
	if (NumPoints > 1)
	{
		FTunnelEntryPoint EndEntry;
		EndEntry.SplinePointIndex = NumPoints - 1;
		EndEntry.Location = TunnelSpline->GetLocationAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::World);
		EndEntry.EntryRotation = TunnelSpline->GetRotationAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::World);
		EndEntry.bIsEntrance = true;
		EndEntry.bIsExit = true;
		EndEntry.EntryTag = TEXT("Entry_End");
		EntryPoints.Add(EndEntry);
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelVolume: Auto-generated %d entry points"), EntryPoints.Num());
	}
}

void ATunnelVolume::ClearEntryPoints()
{
	CleanupTriggerVolumes();
	EntryPoints.Empty();

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelVolume: Cleared all entry points"));
	}
}

// ========================================
// EVENTS
// ========================================

void ATunnelVolume::OnActorEnteredTunnel_Implementation(AActor* Actor, int32 EntryPointIndex)
{
	if (!Actor)
	{
		return;
	}

	if (!ActorsInTunnel.Contains(Actor))
	{
		ActorsInTunnel.Add(Actor);
	}

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelVolume: Actor %s entered tunnel at entry point %d"),
			*Actor->GetName(), EntryPointIndex);
	}
}

void ATunnelVolume::OnActorExitedTunnel_Implementation(AActor* Actor, int32 ExitPointIndex)
{
	if (!Actor)
	{
		return;
	}

	ActorsInTunnel.Remove(Actor);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelVolume: Actor %s exited tunnel at exit point %d"),
			*Actor->GetName(), ExitPointIndex);
	}
}

void ATunnelVolume::OnActorTraversingTunnel_Implementation(AActor* Actor, float DistanceAlongSpline)
{
	// Override in Blueprint for custom behavior
}

// ========================================
// PROTECTED FUNCTIONS
// ========================================

void ATunnelVolume::CreateTriggerVolumes()
{
	if (!GetWorld())
	{
		return;
	}

	CleanupTriggerVolumes();

	for (int32 i = 0; i < EntryPoints.Num(); i++)
	{
		FTunnelEntryPoint& Entry = EntryPoints[i];

		// Create trigger volume
		FName TriggerName = FName(*FString::Printf(TEXT("TunnelTrigger_%d"), i));
		UBoxComponent* Trigger = NewObject<UBoxComponent>(this, TriggerName);

		if (Trigger)
		{
			Trigger->RegisterComponent();
			Trigger->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			Trigger->SetWorldLocation(Entry.Location);
			Trigger->SetWorldRotation(Entry.EntryRotation);
			Trigger->SetBoxExtent(TriggerBoxExtent);
			Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
			Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
			Trigger->SetGenerateOverlapEvents(true);

			// Bind overlap events
			Trigger->OnComponentBeginOverlap.AddDynamic(this, &ATunnelVolume::OnTriggerBeginOverlap);
			Trigger->OnComponentEndOverlap.AddDynamic(this, &ATunnelVolume::OnTriggerEndOverlap);

			Entry.TriggerVolume = Trigger;
			TriggerToEntryPointMap.Add(Trigger, i);

			if (bShowDebug)
			{
				UE_LOG(LogTemp, Log, TEXT("TunnelVolume: Created trigger %d at %s"), i, *Entry.Location.ToString());
			}
		}
	}
}

void ATunnelVolume::CleanupTriggerVolumes()
{
	// Destroy all trigger volumes
	for (auto& Pair : TriggerToEntryPointMap)
	{
		if (Pair.Key)
		{
			Pair.Key->DestroyComponent();
		}
	}

	TriggerToEntryPointMap.Empty();

	// Clear trigger references in entry points
	for (FTunnelEntryPoint& Entry : EntryPoints)
	{
		Entry.TriggerVolume = nullptr;
	}
}

void ATunnelVolume::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	UBoxComponent* TriggerBox = Cast<UBoxComponent>(OverlappedComponent);
	if (!TriggerBox)
	{
		return;
	}

	int32* EntryIndexPtr = TriggerToEntryPointMap.Find(TriggerBox);
	if (!EntryIndexPtr)
	{
		return;
	}

	int32 EntryIndex = *EntryIndexPtr;

	if (!CanActorEnter(OtherActor, EntryIndex))
	{
		return;
	}

	OnActorEnteredTunnel(OtherActor, EntryIndex);
}

void ATunnelVolume::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	// Check if actor is still in any trigger
	bool bStillInTunnel = false;
	for (const auto& Pair : TriggerToEntryPointMap)
	{
		if (Pair.Key && Pair.Key != OverlappedComponent && Pair.Key->IsOverlappingActor(OtherActor))
		{
			bStillInTunnel = true;
			break;
		}
	}

	if (!bStillInTunnel)
	{
		UBoxComponent* TriggerBox = Cast<UBoxComponent>(OverlappedComponent);
		if (TriggerBox)
		{
			int32* ExitIndexPtr = TriggerToEntryPointMap.Find(TriggerBox);
			if (ExitIndexPtr)
			{
				OnActorExitedTunnel(OtherActor, *ExitIndexPtr);
			}
		}
	}
}

#if WITH_EDITORONLY_DATA
void ATunnelVolume::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ATunnelVolume, EntryPoints) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ATunnelVolume, bAutoCreateTriggers) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ATunnelVolume, TriggerBoxExtent))
	{
		if (bAutoCreateTriggers)
		{
			RebuildTriggerVolumes();
		}
	}
}
#endif
