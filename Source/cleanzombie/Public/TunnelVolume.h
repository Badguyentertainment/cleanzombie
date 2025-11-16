// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "TunnelVolume.generated.h"

class UBoxComponent;
class USplineComponent;
class AActor;

/**
 * Tunnel entrance/exit point data
 */
USTRUCT(BlueprintType)
struct FTunnelEntryPoint
{
	GENERATED_BODY()

	/** Location of the entry point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Entry")
	FVector Location;

	/** Direction zombie should face when entering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Entry")
	FRotator EntryRotation;

	/** Trigger volume for this entry point */
	UPROPERTY(BlueprintReadOnly, Category = "Tunnel Entry")
	UBoxComponent* TriggerVolume;

	/** Spline point index this entry connects to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Entry")
	int32 SplinePointIndex = 0;

	/** Is this an entrance (true) or exit only (false)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Entry")
	bool bIsEntrance = true;

	/** Is this an exit (true) or entrance only (false)? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Entry")
	bool bIsExit = true;

	/** Custom tag for this entry point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Entry")
	FName EntryTag;

	FTunnelEntryPoint()
		: Location(FVector::ZeroVector)
		, EntryRotation(FRotator::ZeroRotator)
		, TriggerVolume(nullptr)
		, SplinePointIndex(0)
		, bIsEntrance(true)
		, bIsExit(true)
		, EntryTag(NAME_None)
	{
	}
};

/**
 * Tunnel size configuration
 */
UENUM(BlueprintType)
enum class ETunnelSize : uint8
{
	Small,		// Crawl space, requires crouch/prone
	Medium,		// Standard vent, zombie hunched over
	Large		// Pipe system, zombie can walk normally
};

/**
 * Actor representing a tunnel/vent/pipe system with spline-based navigation
 * Zombies can enter, traverse, and exit at multiple points
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API ATunnelVolume : public AActor
{
	GENERATED_BODY()

public:
	ATunnelVolume();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// COMPONENTS
	// ========================================

	/** Spline defining the tunnel path */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* TunnelSpline;

	/** Root scene component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* TunnelRoot;

	// ========================================
	// TUNNEL CONFIGURATION
	// ========================================

	/** Entry/exit points for this tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	TArray<FTunnelEntryPoint> EntryPoints;

	/** Size category of this tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	ETunnelSize TunnelSize = ETunnelSize::Medium;

	/** Speed multiplier while traversing tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	float TunnelSpeedMultiplier = 0.7f;

	/** Collision capsule radius while in tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	float TunnelCapsuleRadius = 30.0f;

	/** Collision capsule half height while in tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	float TunnelCapsuleHalfHeight = 40.0f;

	/** Scale factor for zombie mesh in tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	float ZombieScaleInTunnel = 0.8f;

	/** Should auto-create triggers for entry points? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	bool bAutoCreateTriggers = true;

	/** Trigger box size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	FVector TriggerBoxExtent = FVector(100, 100, 100);

	/** Tunnel network ID (for grouping connected tunnels) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	FName TunnelNetworkID = TEXT("TunnelNetwork_Default");

	/** Allow zombies to use this tunnel? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	bool bZombiesCanUse = true;

	/** Allow players to use this tunnel? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	bool bPlayersCanUse = false;

	/** Priority for pathfinding (higher = preferred) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Config")
	float PathfindingPriority = 1.0f;

	// ========================================
	// TUNNEL FUNCTIONS
	// ========================================

	/** Can actor enter tunnel at this entry point? */
	UFUNCTION(BlueprintCallable, Category = "Tunnel")
	bool CanActorEnter(AActor* Actor, int32 EntryPointIndex) const;

	/** Get closest entry point to location */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	int32 GetClosestEntryPoint(const FVector& Location, bool bEntrancesOnly = false) const;

	/** Get entry point by index */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	FTunnelEntryPoint GetEntryPoint(int32 Index) const;

	/** Get all entrance points */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	TArray<FTunnelEntryPoint> GetEntrancePoints() const;

	/** Get all exit points */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	TArray<FTunnelEntryPoint> GetExitPoints() const;

	/** Get spline distance from entry to exit */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	float GetTunnelLength(int32 EntryIndex, int32 ExitIndex) const;

	/** Get location along spline at distance */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	FVector GetLocationAtDistance(float Distance) const;

	/** Get rotation along spline at distance */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	FRotator GetRotationAtDistance(float Distance) const;

	/** Get transform along spline at distance */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	FTransform GetTransformAtDistance(float Distance) const;

	/** Get direction along spline at distance */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	FVector GetDirectionAtDistance(float Distance) const;

	/** Get total spline length */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	float GetTotalSplineLength() const;

	/** Find best exit point based on target location */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	int32 FindBestExitPoint(const FVector& TargetLocation) const;

	/** Is location inside tunnel bounds? */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	bool IsLocationInTunnel(const FVector& Location, float Tolerance = 150.0f) const;

	/** Get actors currently in tunnel */
	UFUNCTION(BlueprintPure, Category = "Tunnel")
	TArray<AActor*> GetActorsInTunnel() const { return ActorsInTunnel; }

	// ========================================
	// EDITOR UTILITIES
	// ========================================

	/** Rebuild trigger volumes (editor only) */
	UFUNCTION(CallInEditor, Category = "Tunnel|Editor")
	void RebuildTriggerVolumes();

	/** Add entry point at spline point */
	UFUNCTION(CallInEditor, Category = "Tunnel|Editor")
	void AddEntryPointAtSplinePoint(int32 SplinePointIndex);

	/** Auto-generate entry points at spline ends */
	UFUNCTION(CallInEditor, Category = "Tunnel|Editor")
	void AutoGenerateEntryPoints();

	/** Clear all entry points */
	UFUNCTION(CallInEditor, Category = "Tunnel|Editor")
	void ClearEntryPoints();

	// ========================================
	// EVENTS
	// ========================================

	/** Called when actor enters tunnel */
	UFUNCTION(BlueprintNativeEvent, Category = "Tunnel|Events")
	void OnActorEnteredTunnel(AActor* Actor, int32 EntryPointIndex);
	virtual void OnActorEnteredTunnel_Implementation(AActor* Actor, int32 EntryPointIndex);

	/** Called when actor exits tunnel */
	UFUNCTION(BlueprintNativeEvent, Category = "Tunnel|Events")
	void OnActorExitedTunnel(AActor* Actor, int32 ExitPointIndex);
	virtual void OnActorExitedTunnel_Implementation(AActor* Actor, int32 ExitPointIndex);

	/** Called when actor is traversing tunnel */
	UFUNCTION(BlueprintNativeEvent, Category = "Tunnel|Events")
	void OnActorTraversingTunnel(AActor* Actor, float DistanceAlongSpline);
	virtual void OnActorTraversingTunnel_Implementation(AActor* Actor, float DistanceAlongSpline);

protected:
	/** Actors currently in tunnel */
	UPROPERTY(BlueprintReadOnly, Category = "Tunnel")
	TArray<AActor*> ActorsInTunnel;

	/** Create trigger volumes for entry points */
	void CreateTriggerVolumes();

	/** Cleanup trigger volumes */
	void CleanupTriggerVolumes();

	/** Trigger overlap callbacks */
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Map trigger volumes to entry point indices */
	UPROPERTY()
	TMap<UBoxComponent*, int32> TriggerToEntryPointMap;

	/** Debug visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Debug")
	bool bShowDebug = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Debug")
	bool bDrawSpline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Debug")
	bool bDrawEntryPoints = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel|Debug")
	bool bDrawTriggers = true;

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
