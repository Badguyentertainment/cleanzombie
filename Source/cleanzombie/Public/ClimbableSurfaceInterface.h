// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ClimbableSurfaceInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UClimbableSurfaceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for actors that can be climbed on
 * Implement this in Blueprint or C++ to mark surfaces as climbable
 */
class CLEANZOMBIE_API IClimbableSurfaceInterface
{
	GENERATED_BODY()

public:
	/**
	 * Check if this surface can be climbed at the given location
	 * @param Location - World location to check
	 * @param ClimbingCharacter - Character attempting to climb
	 * @return true if climbable at this location
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Climbable Surface")
	bool CanBeClimbed(const FVector& Location, ACharacter* ClimbingCharacter);

	/**
	 * Get the climb speed multiplier for this surface
	 * @return Speed multiplier (1.0 = normal speed, 0.5 = half speed, etc.)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Climbable Surface")
	float GetClimbSpeedMultiplier();

	/**
	 * Called when a character starts climbing this surface
	 * @param ClimbingCharacter - Character that started climbing
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Climbable Surface")
	void OnClimbingStarted(ACharacter* ClimbingCharacter);

	/**
	 * Called when a character stops climbing this surface
	 * @param ClimbingCharacter - Character that stopped climbing
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Climbable Surface")
	void OnClimbingStopped(ACharacter* ClimbingCharacter);

	/**
	 * Should this surface be highlighted or marked for AI pathfinding?
	 * @return true if surface should be considered for AI navigation
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Climbable Surface")
	bool IsAIClimbable();
};
