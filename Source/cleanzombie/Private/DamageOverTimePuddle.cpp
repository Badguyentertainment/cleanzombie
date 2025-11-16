// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageOverTimePuddle.h"
#include "StatusEffectComponent.h"
#include "Components/DecalComponent.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ADamageOverTimePuddle::ADamageOverTimePuddle()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	DamageTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("DamageTrigger"));
	RootComponent = DamageTrigger;
	DamageTrigger->SetBoxExtent(FVector(150, 150, 50));
	DamageTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DamageTrigger->SetGenerateOverlapEvents(true);

	// Create decal
	PuddleDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("PuddleDecal"));
	PuddleDecal->SetupAttachment(RootComponent);
	PuddleDecal->DecalSize = FVector(16.0f, 150.0f, 150.0f);
	PuddleDecal->SetRelativeRotation(FRotator(-90, 0, 0)); // Point down

	// Create particle effect
	PuddleEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PuddleEffect"));
	PuddleEffect->SetupAttachment(RootComponent);
	PuddleEffect->bAutoActivate = true;

	// Create audio component
	SizzleAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("SizzleAudio"));
	SizzleAudio->SetupAttachment(RootComponent);
	SizzleAudio->bAutoActivate = false;
}

void ADamageOverTimePuddle::BeginPlay()
{
	Super::BeginPlay();

	// Bind overlap events
	DamageTrigger->OnComponentBeginOverlap.AddDynamic(this, &ADamageOverTimePuddle::OnTriggerBeginOverlap);
	DamageTrigger->OnComponentEndOverlap.AddDynamic(this, &ADamageOverTimePuddle::OnTriggerEndOverlap);

	// Apply visual settings
	if (DecalMaterial && PuddleDecal)
	{
		PuddleDecal->SetDecalMaterial(DecalMaterial);
	}

	if (ParticleTemplate && PuddleEffect)
	{
		PuddleEffect->SetTemplate(ParticleTemplate);
	}

	// Start sizzle sound
	if (SizzleSound && SizzleAudio)
	{
		SizzleAudio->SetSound(SizzleSound);
		SizzleAudio->Play();
	}

	// Set puddle size
	DamageTrigger->SetBoxExtent(FVector(PuddleRadius, PuddleRadius, 50.0f));
	PuddleDecal->DecalSize = FVector(16.0f, PuddleRadius, PuddleRadius);

	// Start damage tick timer
	GetWorld()->GetTimerManager().SetTimer(
		DamageTickTimer,
		this,
		&ADamageOverTimePuddle::ApplyDamageToActorsInPuddle,
		TickInterval,
		true
	);

	// Start lifetime timer
	GetWorld()->GetTimerManager().SetTimer(
		LifetimeTimer,
		this,
		&ADamageOverTimePuddle::StartFadeOut,
		PuddleDuration - FadeOutDuration,
		false
	);
}

void ADamageOverTimePuddle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Handle fade out
	if (bIsFadingOut && FadeOutDuration > 0.0f)
	{
		FadeAlpha -= DeltaTime / FadeOutDuration;
		FadeAlpha = FMath::Max(FadeAlpha, 0.0f);

		// Update decal opacity
		if (PuddleDecal)
		{
			// Create dynamic material instance if needed
			UMaterialInstanceDynamic* DynMat = Cast<UMaterialInstanceDynamic>(PuddleDecal->GetDecalMaterial());
			if (!DynMat && DecalMaterial)
			{
				DynMat = UMaterialInstanceDynamic::Create(DecalMaterial, this);
				PuddleDecal->SetDecalMaterial(DynMat);
			}

			if (DynMat)
			{
				DynMat->SetScalarParameterValue(TEXT("Opacity"), FadeAlpha);
			}
		}

		// Fade audio
		if (SizzleAudio)
		{
			SizzleAudio->SetVolumeMultiplier(FadeAlpha);
		}

		// Destroy when fully faded
		if (FadeAlpha <= 0.0f)
		{
			OnPuddleFadedOut();
			Destroy();
		}
	}
}

// ========================================
// PUDDLE FUNCTIONS
// ========================================

void ADamageOverTimePuddle::InitializePuddle(float Damage, float Duration, float Radius, FName StatusEffect)
{
	DamagePerTick = Damage;
	PuddleDuration = Duration;
	PuddleRadius = Radius;
	StatusEffectTag = StatusEffect;

	// Update trigger size
	if (DamageTrigger)
	{
		DamageTrigger->SetBoxExtent(FVector(Radius, Radius, 50.0f));
	}

	// Update decal size
	if (PuddleDecal)
	{
		PuddleDecal->DecalSize = FVector(16.0f, Radius, Radius);
	}
}

void ADamageOverTimePuddle::ApplyDamageToActorsInPuddle()
{
	if (ActorsInPuddle.Num() == 0)
	{
		return;
	}

	for (AActor* Actor : ActorsInPuddle)
	{
		if (!Actor || !IsValid(Actor))
		{
			continue;
		}

		// Apply damage
		FPointDamageEvent DamageEvent;
		DamageEvent.Damage = DamagePerTick;

		Actor->TakeDamage(DamagePerTick, DamageEvent, nullptr, PuddleOwner);

		// Apply/refresh status effect using StatusEffectComponent if available
		UStatusEffectComponent* StatusComp = Actor->FindComponentByClass<UStatusEffectComponent>();
		if (StatusComp && !StatusEffectTag.IsNone())
		{
			// Map tag name to effect type
			EStatusEffectType EffectType = EStatusEffectType::None;

			if (StatusEffectTag == TEXT("Poison"))
			{
				EffectType = EStatusEffectType::Poison;
			}
			else if (StatusEffectTag == TEXT("Acid"))
			{
				EffectType = EStatusEffectType::Acid;
			}
			else if (StatusEffectTag == TEXT("Fire"))
			{
				EffectType = EStatusEffectType::Fire;
			}
			else if (StatusEffectTag == TEXT("Slow"))
			{
				EffectType = EStatusEffectType::Slowing;
			}
			else if (StatusEffectTag == TEXT("Blind"))
			{
				EffectType = EStatusEffectType::Blinding;
			}

			if (EffectType != EStatusEffectType::None)
			{
				// Apply status effect (will stack or refresh duration)
				StatusComp->ApplyStatusEffect(EffectType, StatusEffectStrength, TickInterval * 2.0f, PuddleOwner);
			}
		}
		else if (!StatusEffectTag.IsNone())
		{
			// Fallback: Use simple tag system if no StatusEffectComponent
			if (!Actor->Tags.Contains(StatusEffectTag))
			{
				Actor->Tags.AddUnique(StatusEffectTag);
			}
		}
	}
}

void ADamageOverTimePuddle::StartFadeOut()
{
	if (bIsFadingOut)
	{
		return;
	}

	bIsFadingOut = true;

	// Stop damage ticks
	GetWorld()->GetTimerManager().ClearTimer(DamageTickTimer);

	// Set destruction timer
	SetLifeSpan(FadeOutDuration + 0.5f);
}

TArray<AActor*> ADamageOverTimePuddle::GetActorsInPuddle() const
{
	return ActorsInPuddle;
}

// ========================================
// EVENTS
// ========================================

void ADamageOverTimePuddle::OnActorEnteredPuddle_Implementation(AActor* Actor)
{
	// Override in Blueprint
}

void ADamageOverTimePuddle::OnActorExitedPuddle_Implementation(AActor* Actor)
{
	// Override in Blueprint
	// Remove status effect tag when leaving
	if (Actor && !StatusEffectTag.IsNone())
	{
		Actor->Tags.Remove(StatusEffectTag);
	}
}

void ADamageOverTimePuddle::OnPuddleFadedOut_Implementation()
{
	// Override in Blueprint
	// Remove all status effects
	for (AActor* Actor : ActorsInPuddle)
	{
		if (Actor && !StatusEffectTag.IsNone())
		{
			Actor->Tags.Remove(StatusEffectTag);
		}
	}
}

// ========================================
// OVERLAP CALLBACKS
// ========================================

void ADamageOverTimePuddle::OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == PuddleOwner)
	{
		return;
	}

	if (!ActorsInPuddle.Contains(OtherActor))
	{
		ActorsInPuddle.Add(OtherActor);
		OnActorEnteredPuddle(OtherActor);
	}
}

void ADamageOverTimePuddle::OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
	{
		return;
	}

	if (ActorsInPuddle.Contains(OtherActor))
	{
		ActorsInPuddle.Remove(OtherActor);
		OnActorExitedPuddle(OtherActor);
	}
}
