// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DamageOverTimePuddle.generated.h"

/**
 * Puddle that applies damage over time to actors standing in it
 * Created by projectile impacts for area denial
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API ADamageOverTimePuddle : public AActor
{
	GENERATED_BODY()

public:
	ADamageOverTimePuddle();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// COMPONENTS
	// ========================================

	/** Decal component for visual */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puddle")
	class UDecalComponent* PuddleDecal;

	/** Trigger volume for damage */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puddle")
	class UBoxComponent* DamageTrigger;

	/** Particle effect */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puddle")
	class UParticleSystemComponent* PuddleEffect;

	/** Audio component for sizzle sound */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Puddle")
	class UAudioComponent* SizzleAudio;

	// ========================================
	// PUDDLE CONFIGURATION
	// ========================================

	/** Damage per tick */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Damage")
	float DamagePerTick = 5.0f;

	/** Time between damage ticks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Damage")
	float TickInterval = 0.5f;

	/** Puddle lifetime */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Lifetime")
	float PuddleDuration = 10.0f;

	/** Fade out time before destruction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Lifetime")
	float FadeOutDuration = 2.0f;

	/** Puddle radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Size")
	float PuddleRadius = 150.0f;

	/** Status effect to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Status")
	FName StatusEffectTag;

	/** Status effect strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Status")
	float StatusEffectStrength = 1.0f;

	/** Decal material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Visuals")
	class UMaterialInterface* DecalMaterial;

	/** Particle system template */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Visuals")
	class UParticleSystem* ParticleTemplate;

	/** Sizzle sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Puddle|Audio")
	class USoundBase* SizzleSound;

	/** Who created this puddle */
	UPROPERTY(BlueprintReadWrite, Category = "Puddle")
	AActor* PuddleOwner;

	// ========================================
	// PUDDLE FUNCTIONS
	// ========================================

	/** Initialize puddle with settings */
	UFUNCTION(BlueprintCallable, Category = "Puddle")
	void InitializePuddle(float Damage, float Duration, float Radius, FName StatusEffect);

	/** Apply damage to actors in puddle */
	UFUNCTION(BlueprintCallable, Category = "Puddle")
	void ApplyDamageToActorsInPuddle();

	/** Start fade out */
	UFUNCTION(BlueprintCallable, Category = "Puddle")
	void StartFadeOut();

	/** Get actors currently in puddle */
	UFUNCTION(BlueprintPure, Category = "Puddle")
	TArray<AActor*> GetActorsInPuddle() const;

	// ========================================
	// EVENTS
	// ========================================

	/** Called when actor enters puddle */
	UFUNCTION(BlueprintNativeEvent, Category = "Puddle|Events")
	void OnActorEnteredPuddle(AActor* Actor);
	virtual void OnActorEnteredPuddle_Implementation(AActor* Actor);

	/** Called when actor exits puddle */
	UFUNCTION(BlueprintNativeEvent, Category = "Puddle|Events")
	void OnActorExitedPuddle(AActor* Actor);
	virtual void OnActorExitedPuddle_Implementation(AActor* Actor);

	/** Called when puddle fades out */
	UFUNCTION(BlueprintNativeEvent, Category = "Puddle|Events")
	void OnPuddleFadedOut();
	virtual void OnPuddleFadedOut_Implementation();

protected:
	/** Overlap callbacks */
	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Damage tick timer */
	FTimerHandle DamageTickTimer;

	/** Lifetime timer */
	FTimerHandle LifetimeTimer;

	/** Actors currently in puddle */
	UPROPERTY()
	TArray<AActor*> ActorsInPuddle;

	/** Is puddle fading out? */
	bool bIsFadingOut = false;

	/** Fade alpha (1.0 = full, 0.0 = invisible) */
	float FadeAlpha = 1.0f;
};
