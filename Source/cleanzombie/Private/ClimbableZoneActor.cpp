// Copyright Epic Games, Inc. All Rights Reserved.

#include "ClimbableZoneActor.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

AClimbableZoneActor::AClimbableZoneActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create climbable volume
	ClimbableVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ClimbableVolume"));
	RootComponent = ClimbableVolume;

	ClimbableVolume->SetBoxExtent(FVector(200.0f, 200.0f, 200.0f));
	ClimbableVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ClimbableVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	ClimbableVolume->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	// Make it visible in editor
	ClimbableVolume->SetHiddenInGame(true);
	ClimbableVolume->ShapeColor = FColor::Cyan;

	// Enable replication
	bReplicates = true;
}

void AClimbableZoneActor::BeginPlay()
{
	Super::BeginPlay();
}

void AClimbableZoneActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug visualization
	if (bShowDebug && bIsActive)
	{
		FVector Origin, Extent;
		GetActorBounds(true, Origin, Extent);

		DrawDebugBox(
			GetWorld(),
			Origin,
			Extent,
			FQuat::Identity,
			FColor::Cyan,
			false,
			-1.0f,
			0,
			2.0f
		);

		// Draw text showing active climbers
		if (ClimbingCharacters.Num() > 0)
		{
			FString DebugText = FString::Printf(TEXT("Climbers: %d"), ClimbingCharacters.Num());
			DrawDebugString(
				GetWorld(),
				Origin + FVector(0, 0, Extent.Z),
				DebugText,
				nullptr,
				FColor::Yellow,
				0.0f,
				true
			);
		}
	}
}

// ========================================
// INTERFACE IMPLEMENTATION
// ========================================

bool AClimbableZoneActor::CanBeClimbed_Implementation(const FVector& Location, ACharacter* ClimbingCharacter)
{
	if (!bIsActive)
	{
		return false;
	}

	return IsLocationInZone(Location);
}

float AClimbableZoneActor::GetClimbSpeedMultiplier_Implementation()
{
	return ClimbSpeedMultiplier;
}

void AClimbableZoneActor::OnClimbingStarted_Implementation(ACharacter* ClimbingCharacter)
{
	if (ClimbingCharacter && !ClimbingCharacters.Contains(ClimbingCharacter))
	{
		ClimbingCharacters.Add(ClimbingCharacter);

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("ClimbableZone: %s started climbing"), *ClimbingCharacter->GetName());
		}
	}
}

void AClimbableZoneActor::OnClimbingStopped_Implementation(ACharacter* ClimbingCharacter)
{
	if (ClimbingCharacter)
	{
		ClimbingCharacters.Remove(ClimbingCharacter);

		if (bShowDebug)
		{
			UE_LOG(LogTemp, Log, TEXT("ClimbableZone: %s stopped climbing"), *ClimbingCharacter->GetName());
		}
	}
}

bool AClimbableZoneActor::IsAIClimbable_Implementation()
{
	return bAIClimbable && bIsActive;
}

// ========================================
// FUNCTIONS
// ========================================

void AClimbableZoneActor::SetActive(bool bActive)
{
	bIsActive = bActive;

	if (!bIsActive)
	{
		// Clear all climbing characters
		ClimbingCharacters.Empty();
	}
}

bool AClimbableZoneActor::IsLocationInZone(const FVector& Location) const
{
	if (!ClimbableVolume)
	{
		return false;
	}

	FVector LocalLocation = ClimbableVolume->GetComponentTransform().InverseTransformPosition(Location);
	FVector BoxExtent = ClimbableVolume->GetScaledBoxExtent();

	return FMath::Abs(LocalLocation.X) <= BoxExtent.X &&
		FMath::Abs(LocalLocation.Y) <= BoxExtent.Y &&
		FMath::Abs(LocalLocation.Z) <= BoxExtent.Z;
}
