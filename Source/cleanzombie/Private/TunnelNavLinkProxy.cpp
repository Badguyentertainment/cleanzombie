// Copyright Epic Games, Inc. All Rights Reserved.

#include "TunnelNavLinkProxy.h"
#include "TunnelVolume.h"
#include "TunnelNavigationAbility.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavLinkCustomInterface.h"

ATunnelNavLinkProxy::ATunnelNavLinkProxy()
{
	bSmartLinkIsRelevant = true;
}

void ATunnelNavLinkProxy::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoCreateLinks && LinkedTunnel)
	{
		CreateTunnelNavLinks();
	}
}

void ATunnelNavLinkProxy::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bAutoCreateLinks && LinkedTunnel)
	{
		CreateTunnelNavLinks();
	}
}

// ========================================
// NAV LINK FUNCTIONS
// ========================================

void ATunnelNavLinkProxy::CreateTunnelNavLinks()
{
	if (!LinkedTunnel)
	{
		UE_LOG(LogTemp, Warning, TEXT("TunnelNavLinkProxy: No linked tunnel set"));
		return;
	}

	ClearNavLinks();

	TArray<FTunnelEntryPoint> EntryPoints = LinkedTunnel->GetEntrancePoints();
	TArray<FTunnelEntryPoint> ExitPoints = LinkedTunnel->GetExitPoints();

	if (EntryPoints.Num() == 0 || ExitPoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TunnelNavLinkProxy: Tunnel has no valid entry/exit points"));
		return;
	}

	// Create links from each entrance to each exit
	for (const FTunnelEntryPoint& Entry : EntryPoints)
	{
		for (const FTunnelEntryPoint& Exit : ExitPoints)
		{
			if (&Entry == &Exit)
			{
				continue; // Skip same point
			}

			CreateLink(Entry, Exit);

			// Create reverse link if bidirectional
			if (bBidirectional && Entry.bIsExit && Exit.bIsEntrance)
			{
				CreateLink(Exit, Entry);
			}
		}
	}

	// Refresh navigation
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		NavSys->UpdateNavOctreeAll();
	}

	UE_LOG(LogTemp, Log, TEXT("TunnelNavLinkProxy: Created %d nav links for tunnel %s"),
		NumCreatedLinks, *LinkedTunnel->GetName());
}

void ATunnelNavLinkProxy::ClearNavLinks()
{
	// Clear point links
	PointLinks.Empty();
	CreatedLinkIDs.Empty();
	NumCreatedLinks = 0;

	UE_LOG(LogTemp, Log, TEXT("TunnelNavLinkProxy: Cleared all nav links"));
}

void ATunnelNavLinkProxy::RefreshNavLinks()
{
	CreateTunnelNavLinks();
}

void ATunnelNavLinkProxy::CreateLink(const FTunnelEntryPoint& StartPoint, const FTunnelEntryPoint& EndPoint)
{
	if (!LinkedTunnel)
	{
		return;
	}

	// Calculate link cost based on tunnel length
	float TunnelLength = LinkedTunnel->GetTotalSplineLength();
	float LinkCost = TunnelLength * LinkCostMultiplier;

	// Create nav link simple struct
	FNavigationLink NewLink;
	NewLink.Left = StartPoint.Location;
	NewLink.Right = EndPoint.Location;
	NewLink.Direction = bBidirectional ? ENavLinkDirection::BothWays : ENavLinkDirection::LeftToRight;
	NewLink.bUseSnapHeight = false;
	NewLink.SnapRadius = 100.0f;

	// Add to point links
	PointLinks.Add(NewLink);
	NumCreatedLinks++;

	if (bUseSmartLink)
	{
		SetSmartLinkEnabled(true);
	}
}

void ATunnelNavLinkProxy::OnSmartLinkReceived_Implementation(AActor* Agent, const FVector& DestinationPoint)
{
	if (!Agent || !LinkedTunnel)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("TunnelNavLinkProxy: Agent %s requesting tunnel traversal"), *Agent->GetName());

	// Find tunnel navigation ability on agent
	UTunnelNavigationAbility* TunnelAbility = Agent->FindComponentByClass<UTunnelNavigationAbility>();
	if (!TunnelAbility)
	{
		UE_LOG(LogTemp, Warning, TEXT("TunnelNavLinkProxy: Agent %s has no TunnelNavigationAbility"), *Agent->GetName());
		return;
	}

	// Find closest entry point to agent
	int32 EntryIndex = LinkedTunnel->GetClosestEntryPoint(Agent->GetActorLocation(), true);
	if (EntryIndex < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TunnelNavLinkProxy: No valid entry point found"));
		return;
	}

	// Enter tunnel
	bool bSuccess = TunnelAbility->EnterTunnel(LinkedTunnel, EntryIndex, DestinationPoint);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("TunnelNavLinkProxy: Agent %s entered tunnel at entry %d"), *Agent->GetName(), EntryIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TunnelNavLinkProxy: Agent %s failed to enter tunnel"), *Agent->GetName());
	}
}

#if WITH_EDITORONLY_DATA
void ATunnelNavLinkProxy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(ATunnelNavLinkProxy, LinkedTunnel) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ATunnelNavLinkProxy, bAutoCreateLinks) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(ATunnelNavLinkProxy, bBidirectional))
	{
		if (bAutoCreateLinks && LinkedTunnel)
		{
			CreateTunnelNavLinks();
		}
	}
}
#endif
