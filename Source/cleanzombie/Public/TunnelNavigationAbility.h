// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "TunnelVolume.h"
#include "TunnelNavigationAbility.generated.h"

class ATunnelVolume;
class UCapsuleComponent;
class UCharacterMovementComponent;

/**
 * Tunnel traversal state
 */
UENUM(BlueprintType)
enum class ETunnelTraversalState : uint8
{
	None,			// Not in tunnel
	Entering,		// Entering tunnel (playing entry animation)
	Traversing,		// Moving through tunnel along spline
	Exiting			// Exiting tunnel (playing exit animation)
};

/**
 * Tunnel navigation data for zombies
 */
USTRUCT(BlueprintType)
struct FTunnelNavigationData
{
	GENERATED_BODY()

	/** Current tunnel being traversed */
	UPROPERTY(BlueprintReadOnly)
	ATunnelVolume* CurrentTunnel = nullptr;

	/** Entry point index */
	UPROPERTY(BlueprintReadOnly)
	int32 EntryPointIndex = -1;

	/** Target exit point index */
	UPROPERTY(BlueprintReadOnly)
	int32 ExitPointIndex = -1;

	/** Current distance along spline */
	UPROPERTY(BlueprintReadOnly)
	float CurrentDistance = 0.0f;

	/** Total distance to travel */
	UPROPERTY(BlueprintReadOnly)
	float TotalDistance = 0.0f;

	/** Current traversal state */
	UPROPERTY(BlueprintReadOnly)
	ETunnelTraversalState TraversalState = ETunnelTraversalState::None;

	/** Time spent traversing */
	UPROPERTY(BlueprintReadOnly)
	float TraversalTime = 0.0f;

	/** Target location when exiting tunnel */
	UPROPERTY(BlueprintReadOnly)
	FVector ExitTargetLocation = FVector::ZeroVector;

	FTunnelNavigationData()
		: CurrentTunnel(nullptr)
		, EntryPointIndex(-1)
		, ExitPointIndex(-1)
		, CurrentDistance(0.0f)
		, TotalDistance(0.0f)
		, TraversalState(ETunnelTraversalState::None)
		, TraversalTime(0.0f)
		, ExitTargetLocation(FVector::ZeroVector)
	{
	}
};

/**
 * Ability component for tunnel navigation
 * Allows zombies to enter, traverse, and exit tunnel systems
 * Integrates with AI pathfinding and movement systems
 */
UCLASS(ClassGroup = (ZombieAbilities), meta = (BlueprintSpawnableComponent))
class CLEANZOMBIE_API UTunnelNavigationAbility : public UZombieAbilityComponent
{
	GENERATED_BODY()

public:
	UTunnelNavigationAbility();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ========================================
	// ABILITY OVERRIDES
	// ========================================

	virtual void InitializeAbility_Implementation() override;
	virtual bool ActivateAbility_Implementation() override;
	virtual void DeactivateAbility_Implementation() override;
	virtual void UpdateAbility_Implementation(float DeltaTime) override;
	virtual bool CanActivate_Implementation() const override;

	// ========================================
	// TUNNEL NAVIGATION
	// ========================================

	/** Enter a tunnel at specific entry point */
	UFUNCTION(BlueprintCallable, Category = "Tunnel Navigation")
	bool EnterTunnel(ATunnelVolume* Tunnel, int32 EntryPointIndex, const FVector& TargetLocation);

	/** Exit current tunnel */
	UFUNCTION(BlueprintCallable, Category = "Tunnel Navigation")
	void ExitTunnel();

	/** Abort tunnel traversal */
	UFUNCTION(BlueprintCallable, Category = "Tunnel Navigation")
	void AbortTunnelTraversal();

	/** Is zombie currently in a tunnel? */
	UFUNCTION(BlueprintPure, Category = "Tunnel Navigation")
	bool IsInTunnel() const;

	/** Is zombie currently traversing a tunnel? */
	UFUNCTION(BlueprintPure, Category = "Tunnel Navigation")
	bool IsTraversingTunnel() const;

	/** Get current tunnel */
	UFUNCTION(BlueprintPure, Category = "Tunnel Navigation")
	ATunnelVolume* GetCurrentTunnel() const { return TunnelData.CurrentTunnel; }

	/** Get tunnel navigation data */
	UFUNCTION(BlueprintPure, Category = "Tunnel Navigation")
	FTunnelNavigationData GetTunnelData() const { return TunnelData; }

	/** Get traversal progress (0-1) */
	UFUNCTION(BlueprintPure, Category = "Tunnel Navigation")
	float GetTraversalProgress() const;

	/** Find nearest tunnel entrance */
	UFUNCTION(BlueprintCallable, Category = "Tunnel Navigation")
	ATunnelVolume* FindNearestTunnel(float MaxDistance = 2000.0f, int32& OutEntryPointIndex) const;

	/** Can zombie use this tunnel? */
	UFUNCTION(BlueprintPure, Category = "Tunnel Navigation")
	bool CanUseTunnel(ATunnelVolume* Tunnel) const;

	/** Find best tunnel to reach target location */
	UFUNCTION(BlueprintCallable, Category = "Tunnel Navigation")
	ATunnelVolume* FindBestTunnelToTarget(const FVector& TargetLocation, float MaxSearchRadius = 3000.0f,
		int32& OutEntryIndex, int32& OutExitIndex) const;

	// ========================================
	// CONFIGURATION
	// ========================================

	/** Movement speed while traversing tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	float TunnelMovementSpeed = 200.0f;

	/** Time to enter tunnel (animation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	float EnterDuration = 0.5f;

	/** Time to exit tunnel (animation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	float ExitDuration = 0.5f;

	/** Should restore collision after exiting? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	bool bRestoreCollisionOnExit = true;

	/** Should restore mesh scale after exiting? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	bool bRestoreMeshScaleOnExit = true;

	/** Enable automatic tunnel detection? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	bool bAutoDetectTunnels = true;

	/** Detection radius for automatic tunnel finding */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	float TunnelDetectionRadius = 1000.0f;

	/** Should use tunnels to reach targets? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	bool bUseTunnelsForPathfinding = true;

	/** Prefer tunnels over normal paths? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	bool bPreferTunnels = false;

	/** Maximum detour distance to use tunnel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Navigation|Config")
	float MaxTunnelDetourDistance = 500.0f;

	// ========================================
	// EVENTS
	// ========================================

	/** Called when entering tunnel */
	UFUNCTION(BlueprintNativeEvent, Category = "Tunnel Navigation|Events")
	void OnTunnelEntered(ATunnelVolume* Tunnel, int32 EntryPointIndex);
	virtual void OnTunnelEntered_Implementation(ATunnelVolume* Tunnel, int32 EntryPointIndex);

	/** Called when exiting tunnel */
	UFUNCTION(BlueprintNativeEvent, Category = "Tunnel Navigation|Events")
	void OnTunnelExited(ATunnelVolume* Tunnel, int32 ExitPointIndex);
	virtual void OnTunnelExited_Implementation(ATunnelVolume* Tunnel, int32 ExitPointIndex);

	/** Called while traversing tunnel */
	UFUNCTION(BlueprintNativeEvent, Category = "Tunnel Navigation|Events")
	void OnTunnelTraversing(float Progress);
	virtual void OnTunnelTraversing_Implementation(float Progress);

	/** Called when tunnel traversal is aborted */
	UFUNCTION(BlueprintNativeEvent, Category = "Tunnel Navigation|Events")
	void OnTunnelAborted();
	virtual void OnTunnelAborted_Implementation();

protected:
	/** Current tunnel navigation data */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Tunnel Navigation")
	FTunnelNavigationData TunnelData;

	/** Original capsule radius (before entering tunnel) */
	UPROPERTY()
	float OriginalCapsuleRadius = 0.0f;

	/** Original capsule half height (before entering tunnel) */
	UPROPERTY()
	float OriginalCapsuleHalfHeight = 0.0f;

	/** Original mesh scale (before entering tunnel) */
	UPROPERTY()
	FVector OriginalMeshScale = FVector::OneVector;

	/** Cached capsule component */
	UPROPERTY()
	UCapsuleComponent* CachedCapsule = nullptr;

	/** Cached movement component */
	UPROPERTY()
	UCharacterMovementComponent* CachedMovement = nullptr;

	/** Process tunnel entry */
	void ProcessTunnelEntry(float DeltaTime);

	/** Process tunnel traversal */
	void ProcessTunnelTraversal(float DeltaTime);

	/** Process tunnel exit */
	void ProcessTunnelExit(float DeltaTime);

	/** Modify zombie for tunnel traversal */
	void ModifyForTunnel(ATunnelVolume* Tunnel);

	/** Restore zombie to normal state */
	void RestoreFromTunnel();

	/** Update zombie position along spline */
	void UpdateSplineMovement(float DeltaTime);

	/** Get all tunnels in world */
	TArray<ATunnelVolume*> GetAllTunnelsInWorld() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
