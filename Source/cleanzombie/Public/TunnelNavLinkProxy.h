// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Navigation/NavLinkProxy.h"
#include "TunnelVolume.h"
#include "TunnelNavLinkProxy.generated.h"

/**
 * Custom Nav Link Proxy for tunnel navigation
 * Creates smart nav links between tunnel entrances and exits
 * Allows AI pathfinding to use tunnels as navigation shortcuts
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API ATunnelNavLinkProxy : public ANavLinkProxy
{
	GENERATED_BODY()

public:
	ATunnelNavLinkProxy();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// ========================================
	// TUNNEL LINK CONFIGURATION
	// ========================================

	/** Tunnel volume to create nav links for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Nav Link")
	ATunnelVolume* LinkedTunnel;

	/** Auto-create nav links when tunnel is set? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Nav Link")
	bool bAutoCreateLinks = true;

	/** Should create bidirectional links? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Nav Link")
	bool bBidirectional = true;

	/** Nav link cost multiplier (higher = less preferred) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Nav Link")
	float LinkCostMultiplier = 1.0f;

	/** Should use smart nav link system? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunnel Nav Link")
	bool bUseSmartLink = true;

	// ========================================
	// NAV LINK FUNCTIONS
	// ========================================

	/** Create nav links for linked tunnel */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Tunnel Nav Link")
	void CreateTunnelNavLinks();

	/** Clear all nav links */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "Tunnel Nav Link")
	void ClearNavLinks();

	/** Get number of active nav links */
	UFUNCTION(BlueprintPure, Category = "Tunnel Nav Link")
	int32 GetNumNavLinks() const { return NumCreatedLinks; }

	/** Refresh nav links (recreate) */
	UFUNCTION(BlueprintCallable, Category = "Tunnel Nav Link")
	void RefreshNavLinks();

	// ========================================
	// SMART LINK OVERRIDES
	// ========================================

	/** Called when AI wants to traverse this link */
	UFUNCTION(BlueprintNativeEvent, Category = "Tunnel Nav Link")
	void OnSmartLinkReceived(AActor* Agent, const FVector& DestinationPoint);
	virtual void OnSmartLinkReceived_Implementation(AActor* Agent, const FVector& DestinationPoint);

protected:
	/** Number of links created */
	UPROPERTY(BlueprintReadOnly, Category = "Tunnel Nav Link")
	int32 NumCreatedLinks = 0;

	/** Link IDs for cleanup */
	UPROPERTY()
	TArray<int32> CreatedLinkIDs;

	/** Create link between two entry points */
	void CreateLink(const FTunnelEntryPoint& StartPoint, const FTunnelEntryPoint& EndPoint);

#if WITH_EDITORONLY_DATA
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
