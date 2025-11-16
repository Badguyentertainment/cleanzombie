// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ClimbableSurfaceInterface.h"
#include "ClimbableZoneActor.generated.h"

/**
 * Helper actor for marking zones as climbable
 * Place in level to define climbable areas
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API AClimbableZoneActor : public AActor, public IClimbableSurfaceInterface
{
	GENERATED_BODY()

public:
	AClimbableZoneActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// CLIMBABLE ZONE PROPERTIES
	// ========================================

	/** Visual representation in editor */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Climbable Zone")
	class UBoxComponent* ClimbableVolume;

	/** Is this zone currently active? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbable Zone")
	bool bIsActive = true;

	/** Speed multiplier when climbing this zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbable Zone", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float ClimbSpeedMultiplier = 1.0f;

	/** Can AI use this zone for pathfinding? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbable Zone")
	bool bAIClimbable = true;

	/** Show debug visualization in game? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climbable Zone|Debug")
	bool bShowDebug = false;

	// ========================================
	// INTERFACE IMPLEMENTATION
	// ========================================

	virtual bool CanBeClimbed_Implementation(const FVector& Location, ACharacter* ClimbingCharacter) override;
	virtual float GetClimbSpeedMultiplier_Implementation() override;
	virtual void OnClimbingStarted_Implementation(ACharacter* ClimbingCharacter) override;
	virtual void OnClimbingStopped_Implementation(ACharacter* ClimbingCharacter) override;
	virtual bool IsAIClimbable_Implementation() override;

	// ========================================
	// FUNCTIONS
	// ========================================

	/** Enable/disable this climbable zone */
	UFUNCTION(BlueprintCallable, Category = "Climbable Zone")
	void SetActive(bool bActive);

	/** Check if location is within climbable volume */
	UFUNCTION(BlueprintCallable, Category = "Climbable Zone")
	bool IsLocationInZone(const FVector& Location) const;

private:
	/** Characters currently climbing this zone */
	UPROPERTY()
	TArray<ACharacter*> ClimbingCharacters;
};
