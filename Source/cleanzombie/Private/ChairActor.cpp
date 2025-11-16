// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChairActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"

AChairActor::AChairActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication
	bReplicates = true;

	// Create chair mesh
	ChairMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChairMesh"));
	RootComponent = ChairMesh;
	ChairMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ChairMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// Create sit position
	SitPosition = CreateDefaultSubobject<USceneComponent>(TEXT("SitPosition"));
	SitPosition->SetupAttachment(RootComponent);
	SitPosition->SetRelativeLocation(FVector(0, 0, 50)); // Default sit height

	// Create camera position
	CameraPosition = CreateDefaultSubobject<USceneComponent>(TEXT("CameraPosition"));
	CameraPosition->SetupAttachment(SitPosition);
	CameraPosition->SetRelativeLocation(FVector(0, 0, 60)); // Eye level from sit position

	// Create interaction trigger
	InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
	InteractionTrigger->SetupAttachment(RootComponent);
	InteractionTrigger->SetBoxExtent(FVector(100, 100, 100));
	InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	InteractionTrigger->SetGenerateOverlapEvents(true);
}

void AChairActor::BeginPlay()
{
	Super::BeginPlay();

	// Bind trigger overlap events
	if (InteractionTrigger)
	{
		InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AChairActor::OnTriggerBeginOverlap);
		InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AChairActor::OnTriggerEndOverlap);
	}
}

void AChairActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Apply effects to occupant
	if (CurrentOccupant)
	{
		// Health regeneration
		if (HealthRegenPerSecond > 0.0f)
		{
			// Blueprint can handle this via OnActorSitting event
		}

		// Call Blueprint event
		OnActorSitting(CurrentOccupant, DeltaTime);
	}

	// Debug visualization
	if (bShowDebug)
	{
		FTransform SitTransform = GetSitTransform();
		DrawDebugCoordinateSystem(GetWorld(), SitTransform.GetLocation(), SitTransform.Rotator(),
			50.0f, false, 0.0f, 0, 2.0f);

		FString StatusText = FString::Printf(TEXT("Chair: %s\nOccupied: %s"),
			*UEnum::GetValueAsString(ChairType), CurrentOccupant ? TEXT("Yes") : TEXT("No"));
		DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 100), StatusText,
			nullptr, FColor::Cyan, 0.0f, true);
	}
}

void AChairActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AChairActor, CurrentOccupant);
}

// ========================================
// CHAIR FUNCTIONS
// ========================================

bool AChairActor::CanActorSit(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	// Check if occupied
	if (CurrentOccupant != nullptr)
	{
		return false;
	}

	// Check permissions
	APawn* Pawn = Cast<APawn>(Actor);
	if (Pawn)
	{
		if (Pawn->IsPlayerControlled() && !bPlayersCanUse)
		{
			return false;
		}

		if (!Pawn->IsPlayerControlled() && !bNPCsCanUse)
		{
			return false;
		}
	}

	return true;
}

FTransform AChairActor::GetSitTransform() const
{
	if (!SitPosition)
	{
		return GetActorTransform();
	}

	FTransform Transform;
	Transform.SetLocation(SitPosition->GetComponentLocation() + SitOffset);
	Transform.SetRotation((GetActorRotation() + SitRotation).Quaternion());
	Transform.SetScale3D(FVector::OneVector);

	return Transform;
}

FTransform AChairActor::GetCameraTransform() const
{
	if (!CameraPosition)
	{
		return GetSitTransform();
	}

	return CameraPosition->GetComponentTransform();
}

void AChairActor::SetOccupant(AActor* Actor)
{
	CurrentOccupant = Actor;

	if (Actor)
	{
		OnActorSat(Actor);

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("ChairActor: %s occupied by %s"), *GetName(), *Actor->GetName());
		}
	}
}

void AChairActor::ClearOccupant()
{
	if (CurrentOccupant)
	{
		AActor* PreviousOccupant = CurrentOccupant;
		CurrentOccupant = nullptr;
		OnActorUnsit(PreviousOccupant);

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("ChairActor: %s cleared occupant"), *GetName());
		}
	}
}

// ========================================
// TRIGGER CALLBACKS
// ========================================

void AChairActor::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	ActorsInRange.AddUnique(OtherActor);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("ChairActor: %s entered range"), *OtherActor->GetName());
	}
}

void AChairActor::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	ActorsInRange.Remove(OtherActor);

	if (bShowDebug)
	{
		UE_LOG(LogTemp, Log, TEXT("ChairActor: %s left range"), *OtherActor->GetName());
	}
}
