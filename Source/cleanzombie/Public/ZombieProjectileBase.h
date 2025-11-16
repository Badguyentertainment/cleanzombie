// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "ZombieProjectileBase.generated.h"

/**
 * Projectile type enum
 */
UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Acid UMETA(DisplayName = "Acid Spit"),
	Poison UMETA(DisplayName = "Poison Spit"),
	Slowing UMETA(DisplayName = "Slowing Spit"),
	Blinding UMETA(DisplayName = "Blinding Spit"),
	Explosive UMETA(DisplayName = "Explosive Spit"),
	Fire UMETA(DisplayName = "Fire Spit")
};

/**
 * Projectile configuration data
 */
USTRUCT(BlueprintType)
struct FProjectileConfigData : public FTableRowBase
{
	GENERATED_BODY()

	/** Type of projectile */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EProjectileType ProjectileType = EProjectileType::Acid;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FText DisplayName;

	/** Base damage on direct hit */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Damage")
	float DirectHitDamage = 25.0f;

	/** Splash damage radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Damage")
	float SplashRadius = 150.0f;

	/** Splash damage amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Damage")
	float SplashDamage = 10.0f;

	/** Should create puddle on impact? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Puddle")
	bool bCreatePuddle = true;

	/** Puddle actor class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Puddle")
	TSubclassOf<AActor> PuddleClass;

	/** Puddle duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Puddle")
	float PuddleDuration = 10.0f;

	/** Status effect to apply */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Status")
	FName StatusEffectTag;

	/** Status effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Status")
	float StatusEffectDuration = 5.0f;

	/** Status effect strength (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Status")
	float StatusEffectStrength = 1.0f;

	/** Projectile mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visuals")
	class UStaticMesh* ProjectileMesh;

	/** Projectile trail particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visuals")
	class UParticleSystem* TrailEffect;

	/** Impact particle system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visuals")
	class UParticleSystem* ImpactEffect;

	/** Projectile material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Visuals")
	class UMaterialInterface* ProjectileMaterial;

	/** Impact sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Audio")
	class USoundBase* ImpactSound;

	/** Flight sound (looping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config|Audio")
	class USoundBase* FlightSound;

	FProjectileConfigData()
	{
		DisplayName = FText::FromString(TEXT("Acid Spit"));
	}
};

/**
 * Base projectile class for zombie spit attacks
 * Supports arc trajectory, splash damage, puddle creation, and status effects
 */
UCLASS(Blueprintable)
class CLEANZOMBIE_API AZombieProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AZombieProjectileBase();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ========================================
	// COMPONENTS
	// ========================================

	/** Collision component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class USphereComponent* CollisionSphere;

	/** Mesh component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class UStaticMeshComponent* ProjectileMesh;

	/** Movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class UProjectileMovementComponent* ProjectileMovement;

	/** Trail effect component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class UParticleSystemComponent* TrailEffect;

	/** Audio component for flight sound */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	class UAudioComponent* FlightAudio;

	// ========================================
	// PROJECTILE CONFIGURATION
	// ========================================

	/** Projectile type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Config", Replicated)
	EProjectileType ProjectileType = EProjectileType::Acid;

	/** Configuration data table */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Config")
	class UDataTable* ProjectileConfigTable;

	/** Config row name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Config")
	FName ConfigRowName;

	/** Direct hit damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage", Replicated)
	float DirectHitDamage = 25.0f;

	/** Splash damage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
	float SplashDamage = 10.0f;

	/** Splash radius */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Damage")
	float SplashRadius = 150.0f;

	/** Create puddle on impact? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Puddle")
	bool bCreatePuddle = true;

	/** Puddle class to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Puddle")
	TSubclassOf<AActor> PuddleClass;

	/** Puddle duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Puddle")
	float PuddleDuration = 10.0f;

	/** Status effect tag */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Status")
	FName StatusEffectTag;

	/** Status effect duration */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Status")
	float StatusEffectDuration = 5.0f;

	/** Status effect strength */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Status")
	float StatusEffectStrength = 1.0f;

	/** Impact particle effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	class UParticleSystem* ImpactEffect;

	/** Impact sound */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile|Effects")
	class USoundBase* ImpactSound;

	/** Who fired this projectile */
	UPROPERTY(BlueprintReadWrite, Category = "Projectile", Replicated)
	AActor* ProjectileOwner;

	// ========================================
	// PROJECTILE FUNCTIONS
	// ========================================

	/** Initialize from configuration */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitializeFromConfig(FName RowName);

	/** Initialize from config data */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void ApplyConfiguration(const FProjectileConfigData& Config);

	/** Fire projectile in direction */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void FireInDirection(const FVector& ShootDirection, float Speed = 1000.0f);

	/** Fire projectile with arc to hit location */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	bool FireWithArc(const FVector& TargetLocation, float ArcHeight = 200.0f);

	/** Handle impact */
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile")
	void OnProjectileImpact(const FHitResult& Hit);
	virtual void OnProjectileImpact_Implementation(const FHitResult& Hit);

	/** Apply damage to actors in area */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void ApplySplashDamage(const FVector& Location);

	/** Apply status effect to actor */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void ApplyStatusEffect(AActor* Target);

	/** Spawn puddle at location */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	AActor* SpawnPuddle(const FVector& Location, const FVector& Normal);

	/** Get current configuration */
	UFUNCTION(BlueprintPure, Category = "Projectile")
	FProjectileConfigData GetCurrentConfig() const { return CurrentConfig; }

	// Network replication
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/** Collision callback */
	UFUNCTION()
	void OnCollisionHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	/** Current configuration */
	UPROPERTY()
	FProjectileConfigData CurrentConfig;

	/** Has projectile impacted? */
	bool bHasImpacted = false;
};
