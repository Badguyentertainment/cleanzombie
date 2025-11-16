// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChairActor.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

/**
 * Chair type for different gameplay uses
 */
UENUM(BlueprintType)
enum class EChairType : uint8
{
	Standard,			// Regular chair, no special effects
	MountedGun,			// Chair with mounted weapon
	HealingStation,		// Regenerates health while sitting
	SafeZone,			// Can't be attacked while sitting
	SniperNest,			// Enhanced accuracy and zoom
	CommandPost,		// Access tactical map/controls
	TrapControl,		// Activate traps from chair
	OverwatchStation,	// Enhanced vision/detection
	RepairStation,		// Repair barricades while sitting
	LastStandSeat		// Bonus damage, die sitting like a hero
};

/**
 * Interactive chair actor compatible with Blueprint SitInChairComponent
 * Provides various gameplay bonuses and tactical options
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API AChairActor : public AActor
{
	GENERATED_BODY()

public:
	AChairActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// COMPONENTS
	// ========================================

	/** Chair mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ChairMesh;

	/** Sit position marker */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SitPosition;

	/** Interaction trigger */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* InteractionTrigger;

	/** Camera position for first person view (optional) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* CameraPosition;

	// ========================================
	// CHAIR CONFIGURATION
	// ========================================

	/** Type of chair */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Config")
	EChairType ChairType = EChairType::Standard;

	/** Can be used by players? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Config")
	bool bPlayersCanUse = true;

	/** Can be used by NPCs? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Config")
	bool bNPCsCanUse = false;

	/** Maximum occupants (1 = single seat) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Config")
	int32 MaxOccupants = 1;

	/** Rotation when sitting (relative to chair) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Config")
	FRotator SitRotation = FRotator::ZeroRotator;

	/** Offset from sit position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Config")
	FVector SitOffset = FVector(0, 0, 50);

	/** Interaction prompt text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Config")
	FText InteractionPrompt = FText::FromString(TEXT("Press E to Sit"));

	// ========================================
	// GAMEPLAY MODIFIERS
	// ========================================

	/** Health regeneration per second while sitting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Gameplay")
	float HealthRegenPerSecond = 0.0f;

	/** Damage multiplier while sitting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Gameplay")
	float DamageMultiplier = 1.0f;

	/** Accuracy multiplier while sitting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Gameplay")
	float AccuracyMultiplier = 1.0f;

	/** Invulnerable while sitting? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Gameplay")
	bool bInvulnerableWhileSitting = false;

	/** Vision range multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Gameplay")
	float VisionRangeMultiplier = 1.0f;

	// ========================================
	// CHAIR FUNCTIONS (Blueprint Callable)
	// ========================================

	/** Can actor sit in this chair? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chair")
	bool CanActorSit(AActor* Actor) const;

	/** Get sit transform for actor */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chair")
	FTransform GetSitTransform() const;

	/** Get camera transform (if applicable) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chair")
	FTransform GetCameraTransform() const;

	/** Is chair occupied? */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chair")
	bool IsOccupied() const { return CurrentOccupant != nullptr; }

	/** Get current occupant */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Chair")
	AActor* GetOccupant() const { return CurrentOccupant; }

	/** Set occupant (called by Blueprint SitInChairComponent) */
	UFUNCTION(BlueprintCallable, Category = "Chair")
	void SetOccupant(AActor* Actor);

	/** Clear occupant */
	UFUNCTION(BlueprintCallable, Category = "Chair")
	void ClearOccupant();

	// ========================================
	// EVENTS (Override in Blueprint)
	// ========================================

	/** Called when actor sits down */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chair|Events")
	void OnActorSat(AActor* Actor);

	/** Called when actor stands up */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chair|Events")
	void OnActorUnsit(AActor* Actor);

	/** Called while actor is sitting (ticking) */
	UFUNCTION(BlueprintImplementableEvent, Category = "Chair|Events")
	void OnActorSitting(AActor* Actor, float DeltaTime);

	/** Get interaction prompt for actor */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Chair|Events")
	FText GetInteractionPromptForActor(AActor* InteractingActor) const;

protected:
	/** Current occupant */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Chair")
	AActor* CurrentOccupant = nullptr;

	/** Trigger overlap callbacks */
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Actors in interaction range */
	UPROPERTY(BlueprintReadOnly, Category = "Chair")
	TArray<AActor*> ActorsInRange;

	/** Debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chair|Debug")
	bool bShowDebug = false;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
