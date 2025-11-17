# Status Effect System - Complete Guide

## Overview

The **StatusEffectComponent** is a comprehensive system for applying buffs, debuffs, damage-over-time (DoT), healing-over-time (HoT), and complex status effects to any actor in your game. This system includes:

- **28 Status Effect Types** (15 debuffs, 8 buffs, 5 special effects)
- **Visual Feedback System** (Niagara/Cascade particles, sounds, screen effects)
- **Resistance & Immunity** (configurable protection against specific effects)
- **Combo/Synergy System** (effects can combine for bonus damage or new effects)
- **Cleansing & Curing** (remove effects by type, category, or severity)
- **Contagion/Spreading** (effects can spread to nearby actors)
- **Shield System** (damage absorption buffs)
- **Tiered Severity** (Minor/Moderate/Severe/Critical)

---

## Table of Contents

1. [Quick Start](#quick-start)
2. [Status Effect Types](#status-effect-types)
3. [Visual Feedback](#visual-feedback)
4. [Resistance & Immunity](#resistance--immunity)
5. [Combos & Synergies](#combos--synergies)
6. [Cleansing & Curing](#cleansing--curing)
7. [Contagion System](#contagion-system)
8. [Shield & Buffs](#shield--buffs)
9. [Blueprint Integration](#blueprint-integration)
10. [C++ Usage](#c-usage)
11. [Advanced Features](#advanced-features)

---

## Quick Start

### 1. Add Component to Actor

**In Blueprint:**
1. Open your Character/Zombie blueprint
2. Add Component → Search for "Status Effect Component"
3. Compile and save

**In C++:**
```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
UStatusEffectComponent* StatusEffectComp;

// In constructor
StatusEffectComp = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffectComp"));
```

### 2. Apply a Status Effect

**In Blueprint:**
```
StatusEffectComponent → Apply Status Effect
- Effect Type: Fire
- Strength: 1.0
- Duration: 10.0
- Instigator: Self
```

**In C++:**
```cpp
StatusEffectComp->ApplyStatusEffect(EStatusEffectType::Fire, 1.0f, 10.0f, this);
```

### 3. Check for Effects

**In Blueprint:**
```
StatusEffectComponent → Has Status Effect (Fire) → Branch
```

**In C++:**
```cpp
if (StatusEffectComp->HasStatusEffect(EStatusEffectType::Fire))
{
    // Actor is on fire!
}
```

---

## Status Effect Types

### Damage Over Time (DoT) Debuffs

| Effect | Description | Default Damage/Tick | Stackable | Special Properties |
|--------|-------------|-------------------|-----------|-------------------|
| **Poison** | Nature toxin damage | 5.0 | Yes (3 stacks) | Standard DoT |
| **Acid** | Corrosive damage | 8.0 | Yes (5 stacks) | Armor degradation |
| **Fire** | Burning damage | 10.0 | Yes (3 stacks) | **Contagious** - spreads to nearby actors |
| **Bleeding** | Physical damage | 6.0 | Yes (5 stacks) | **Gets worse with movement** |
| **Electrified** | Electric shock | 7.0 | No | **Can chain to nearby actors** |
| **Corroded** | Metal/armor decay | 4.0 | Yes (3 stacks) | Armor reduction |
| **Diseased** | Plague/infection | 3.0 | Yes (5 stacks) | **Highly contagious** (30% spread chance) |
| **Irradiated** | Radiation poisoning | 8.0 | Yes (10 stacks) | **Prevents all healing** |

### Movement/Control Debuffs

| Effect | Description | Strength | Stackable | Special Properties |
|--------|-------------|----------|-----------|-------------------|
| **Slowing** | Reduces movement speed | 50% reduction | Yes (2 stacks) | Standard slow |
| **Frozen** | Severe movement impairment | 80% reduction | No | **Shatters on damage for bonus damage** |
| **Stun** | Complete immobilization | 100% | No | Can't move or act |
| **Rooted** | Can't move but can attack | Movement 0% | No | Attack still allowed |

### Combat Debuffs

| Effect | Description | Strength | Stackable | Special Properties |
|--------|-------------|----------|-----------|-------------------|
| **Blinding** | Vision impairment | 70% reduction | No | Screen darkening |
| **Weakness** | Reduced damage output | 50% reduction | Yes (2 stacks) | Affects all damage |
| **Vulnerability** | Increased damage taken | 50% increase | Yes (3 stacks) | Take more damage |
| **Disarmed** | Can't use weapons | 100% | No | Zero damage output |

### Special Debuffs

| Effect | Description | Properties |
|--------|-------------|------------|
| **Confused** | Random movement/attacks | No stacking, AI disruption |
| **Cursed** | Multiple negative effects | **Cannot be cleansed** |

### Healing Buffs

| Effect | Description | Healing/Tick | Stackable | Special Properties |
|--------|-------------|-------------|-----------|-------------------|
| **Regeneration** | Health over time | 10.0 | Yes (3 stacks) | Standard HoT |
| **Shielded** | Temporary damage absorption | Variable | No | Absorbs damage before health |
| **Blessed** | Multiple positive effects | - | No | Immunity to curses |

### Combat Buffs

| Effect | Description | Strength | Stackable | Special Properties |
|--------|-------------|----------|-----------|-------------------|
| **DamageBoost** | Increased damage output | 50% increase | Yes (3 stacks) | Affects all damage |
| **Haste** | Increased movement/attack speed | 50% increase | Yes (2 stacks) | Movement multiplier |
| **Invulnerable** | Immune to all damage | 100% | No | **Complete immunity** |

### Utility Buffs

| Effect | Description | Properties |
|--------|-------------|------------|
| **Invisible** | Can't be targeted | No stacking, AI can't detect |
| **Fortified** | Reduced damage taken | 50% reduction, stacks 3x |

---

## Visual Feedback

The system supports full visual and audio feedback for each effect.

### Setting Up Visual Feedback in Blueprint

1. Create a Blueprint deriving from your character
2. Select **StatusEffectComponent**
3. In **Details** panel, find **Effect Combos** array
4. Add visual assets to specific effect types:

```
Config → Enable Visuals: True

For each effect, configure:
- Particle Effect: Cascade particle system (legacy)
- Niagara Effect: Niagara system (preferred)
- Apply Sound: Sound played when effect starts
- Loop Sound: Sound that loops while effect is active
- Effect Color: Tint color for the effect
- Attach Socket Name: Bone/socket to attach particles to
```

### Example: Fire Effect Visuals

```
Fire Effect Visuals:
- Niagara Effect: NS_Fire_Burning
- Apply Sound: SFX_Fire_Ignite
- Loop Sound: SFX_Fire_Loop
- Effect Color: (R:1.0, G:0.3, B:0.0, A:1.0) - Orange
- Attach Socket Name: spine_03
```

### Automatic Visual Management

The system automatically:
- Spawns particles when effect is applied
- Plays sounds
- Cleans up particles when effect expires
- Stops sounds when effect is removed

---

## Resistance & Immunity

### Adding Resistance

**Blueprint:**
```
StatusEffectComponent → Add Resistance
- Effect Type: Fire
- Resistance Amount: 0.5 (50% resistance)
- Reduces Duration: True
- Reduces Strength: True
```

**C++:**
```cpp
// 50% resistance to fire
StatusEffectComp->AddResistance(EStatusEffectType::Fire, 0.5f, true, true);

// Complete immunity (100% resistance)
StatusEffectComp->AddResistance(EStatusEffectType::Poison, 1.0f, true, true);
```

### How Resistance Works

- **Resistance Amount** = 0.0 to 1.0 (0% to 100%)
- **Reduces Duration**: Effect lasts shorter time
- **Reduces Strength**: Effect deals less damage/has weaker impact

Example:
```
Fire DoT = 10 damage/tick, 10 second duration
50% Fire Resistance applied:
- New Damage = 10 * (1 - 0.5) = 5 damage/tick
- New Duration = 10 * (1 - 0.5) = 5 seconds
```

### Checking Immunity

**Blueprint:**
```
StatusEffectComponent → Is Immune To (Fire) → Branch
```

**C++:**
```cpp
if (StatusEffectComp->IsImmuneTo(EStatusEffectType::Fire))
{
    // Completely immune to fire
}
```

### Invulnerable Buff

Applying **Invulnerable** buff makes actor immune to ALL status effects:

```cpp
StatusEffectComp->ApplyStatusEffect(EStatusEffectType::Invulnerable, 1.0f, 5.0f, nullptr);
// Now immune to all effects for 5 seconds
```

---

## Combos & Synergies

Effects can combine to create powerful reactions!

### Built-in Combos

| Combo | Result | Effect |
|-------|--------|--------|
| **Fire + Acid** | Explosion | 100 damage in 500 radius, consumes both effects |
| **Electrified + Slowing** | Chain Lightning | Stuns target, 75 damage in 400 radius |
| **Poison + Weakness** | Diseased | Creates highly contagious disease effect |
| **Frozen + Vulnerability** | Shatter | 150 bonus damage, breaks frozen effect |
| **Bleeding + Fire** | Cauterize | Stops bleeding, 50 bonus damage |

### How Combos Work

Combos trigger automatically when an actor has both required effects active simultaneously.

```
1. Actor gets hit by Fire attack → Fire effect applied
2. Actor gets hit by Acid attack → Acid effect applied
3. COMBO TRIGGERED! → Explosion deals 100 damage in 500 unit radius
4. Both Fire and Acid effects are consumed (removed)
```

### Creating Custom Combos

**In Blueprint:**
1. Select StatusEffectComponent
2. Find **Effect Combos** array in Details
3. Add new element:
   - Effect A: Fire
   - Effect B: Poison
   - Result Effect: Irradiated
   - Bonus Damage: 75.0
   - Combo Radius: 300.0
   - Consume Both Effects: True

**In C++:**
```cpp
// Add custom combo in BeginPlay or constructor
FEffectCombo CustomCombo;
CustomCombo.EffectA = EStatusEffectType::Fire;
CustomCombo.EffectB = EStatusEffectType::Poison;
CustomCombo.ResultEffect = EStatusEffectType::Irradiated;
CustomCombo.BonusDamage = 75.0f;
CustomCombo.ComboRadius = 300.0f;
CustomCombo.bConsumeBothEffects = true;

StatusEffectComp->EffectCombos.Add(CustomCombo);
```

### Disabling Combos

```cpp
StatusEffectComp->bEnableCombos = false;
```

---

## Cleansing & Curing

### Cleanse Specific Effect

**Blueprint:**
```
StatusEffectComponent → Cleanse Effect (Fire)
```

**C++:**
```cpp
StatusEffectComp->CleanseEffect(EStatusEffectType::Fire);
```

### Cleanse All Debuffs

Removes all negative effects (poison, slowing, stun, etc.):

```cpp
StatusEffectComp->CleanseAllDebuffs();
```

### Cleanse All Buffs

Removes all positive effects (regeneration, damage boost, etc.):

```cpp
StatusEffectComp->CleanseAllBuffs();
```

### Cleanse by Severity

Remove effects up to a certain severity level:

```cpp
// Remove only Minor and Moderate effects
StatusEffectComp->CleanseBySeverity(EEffectSeverity::Moderate);
```

**Severity Tiers:**
- **Minor**: Strength 0.0 - 0.33
- **Moderate**: Strength 0.34 - 0.66
- **Severe**: Strength 0.67 - 0.89
- **Critical**: Strength 0.90+

### Uncleansable Effects

**Cursed** effect cannot be cleansed:
```cpp
Effect.bCanBeCleansed = false; // Set in custom effects
```

---

## Contagion System

Certain effects can spread from infected actors to nearby targets!

### Contagious Effects

| Effect | Contagion Radius | Spread Chance | Spread Interval |
|--------|-----------------|---------------|-----------------|
| **Fire** | 200 units | 15% | 2 seconds |
| **Diseased** | 300 units | 30% | 2 seconds |
| **Electrified** | 400 units | 50% | Variable (when triggered) |

### How Contagion Works

```
1. Actor A is infected with Diseased
2. Every 2 seconds, check for nearby actors within 300 units
3. For each nearby actor, 30% chance to spread
4. If spread succeeds, apply Diseased with reduced strength (70%)
   and duration (50%)
5. Spread continues as long as effect is active
```

### Creating Contagious Effects

**In C++:**
```cpp
FStatusEffect CustomEffect;
CustomEffect.EffectType = EStatusEffectType::Poison;
CustomEffect.Strength = 1.0f;
CustomEffect.Duration = 20.0f;
Custom Effect.bIsContagious = true;
CustomEffect.ContagionRadius = 400.0f;
CustomEffect.ContagionChance = 0.4f; // 40% chance
CustomEffect.ContagionTickInterval = 1.5f; // Check every 1.5 seconds

StatusEffectComp->ApplyCustomStatusEffect(CustomEffect);
```

### Disabling Contagion

```cpp
StatusEffectComp->bEnableContagion = false;
```

---

## Shield & Buffs

### Shield System

The **Shielded** effect provides damage absorption before health is damaged.

**Apply Shield:**
```cpp
StatusEffectComp->ApplyShield(100.0f, 10.0f);
// 100 HP shield for 10 seconds
```

**Absorb Damage:**
```cpp
// In your TakeDamage function:
float RemainingDamage = StatusEffectComp->AbsorbDamage(IncomingDamage);

// Apply only remaining damage to health
Health -= RemainingDamage;
```

**Check Shield Amount:**
```cpp
float CurrentShield = StatusEffectComp->GetShieldAmount();
```

### Damage Boost Buff

Increases damage output:

```cpp
StatusEffectComp->ApplyStatusEffect(EStatusEffectType::DamageBoost, 1.0f, 15.0f, nullptr);

// When dealing damage:
float Multiplier = StatusEffectComp->GetDamageOutputMultiplier();
float FinalDamage = BaseDamage * Multiplier;
```

### Haste Buff

Increases movement speed:

```cpp
StatusEffectComp->ApplyStatusEffect(EStatusEffectType::Haste, 1.0f, 10.0f, nullptr);

// In movement update:
float SpeedMultiplier = StatusEffectComp->GetMovementSpeedMultiplier();
float FinalSpeed = BaseSpeed * SpeedMultiplier;
```

### Regeneration (HoT)

Heals over time:

```cpp
StatusEffectComp->ApplyStatusEffect(EStatusEffectType::Regeneration, 1.0f, 20.0f, nullptr);
// Heals 10 HP/second for 20 seconds (default config)
```

**Implementing Healing in C++:**
```cpp
// Override the OnHoTHealingApplied event
void AMyCharacter::OnHoTHealingApplied_Implementation(float Healing, EStatusEffectType EffectType)
{
    Health = FMath::Min(Health + Healing, MaxHealth);
}
```

---

## Blueprint Integration

### Blueprint Events

Override these events in Blueprint to respond to status effects:

1. **On Status Effect Applied**
   - Called when any effect is applied
   - Parameters: Effect (struct)

2. **On Status Effect Removed**
   - Called when effect expires or is removed
   - Parameters: Effect Type

3. **On DoT Damage Dealt**
   - Called every damage tick
   - Parameters: Damage, Effect Type, Instigator

4. **On HoT Healing Applied**
   - Called every healing tick
   - Parameters: Healing, Effect Type

5. **On Combo Triggered**
   - Called when effect combo activates
   - Parameters: Combo (struct)

6. **On Effect Cleansed**
   - Called when effect is cleansed
   - Parameters: Effect Type

7. **On Effect Spread**
   - Called when effect spreads to another actor
   - Parameters: Target Actor, Effect Type

8. **On Shield Absorbed**
   - Called when shield absorbs damage
   - Parameters: Damage Absorbed, Remaining Shield

### Example Blueprint Event Graph

```
Event On Status Effect Applied
├─ Switch on Effect Type
   ├─ Fire:
   │  └─ Print String: "I'm on fire!"
   │  └─ Play Sound 2D: Scream_Sound
   ├─ Poison:
   │  └─ Print String: "Poisoned!"
   │  └─ Play Camera Shake
   └─ Frozen:
      └─ Print String: "Frozen solid!"
      └─ Set Material Parameter: IceOverlay
```

### UI/HUD Integration

**Get All Active Effects:**
```cpp
TArray<FStatusEffect> ActiveEffects = StatusEffectComp->GetAllActiveEffects();

// Display icons for each effect
for (const FStatusEffect& Effect : ActiveEffects)
{
    // Show icon, duration, stacks
    AddEffectIcon(Effect.EffectType, Effect.TimeRemaining, Effect.CurrentStacks);
}
```

**Display Effect Colors:**
```cpp
FStatusEffect Effect = StatusEffectComp->GetStatusEffect(EStatusEffectType::Fire);
FLinearColor EffectColor = Effect.Visuals.EffectColor;
// Use color for UI tint
```

---

## C++ Usage

### Complete Example: Applying and Managing Effects

```cpp
#include "StatusEffectComponent.h"

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Get status effect component
    StatusEffectComp = FindComponentByClass<UStatusEffectComponent>();

    // Set up resistances (armor provides fire resistance)
    if (HasHeavyArmor())
    {
        StatusEffectComp->AddResistance(EStatusEffectType::Fire, 0.7f);
    }

    // Enable debug logging
    StatusEffectComp->bShowDebug = true;
}

void AMyCharacter::OnHitByFireball()
{
    // Apply fire DoT
    StatusEffectComp->ApplyStatusEffect(
        EStatusEffectType::Fire,
        1.5f,    // Strength
        10.0f,   // Duration
        FireballInstigator
    );
}

void AMyCharacter::DrinkAntidotePotion()
{
    // Cleanse all debuffs
    StatusEffectComp->CleanseAllDebuffs();

    // Apply regeneration
    StatusEffectComp->ApplyStatusEffect(
        EStatusEffectType::Regeneration,
        2.0f,    // Double strength
        30.0f,   // 30 seconds
        nullptr
    );
}

float AMyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Apply shield absorption
    ActualDamage = StatusEffectComp->AbsorbDamage(ActualDamage);

    // Check for vulnerability
    if (StatusEffectComp->HasStatusEffect(EStatusEffectType::Vulnerability))
    {
        FStatusEffect VulnEffect = StatusEffectComp->GetStatusEffect(EStatusEffectType::Vulnerability);
        ActualDamage *= (1.0f + VulnEffect.Strength); // Take 50% more damage
    }

    // Check for frozen (shatter mechanic)
    if (StatusEffectComp->HasStatusEffect(EStatusEffectType::Frozen))
    {
        // Frozen shatters on damage - deal bonus damage
        ActualDamage *= 2.0f;
        StatusEffectComp->RemoveStatusEffect(EStatusEffectType::Frozen);

        // Play shatter effect
        PlayShatterEffect();
    }

    Health -= ActualDamage;
    return ActualDamage;
}

void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply movement speed from status effects
    if (StatusEffectComp)
    {
        float SpeedMultiplier = StatusEffectComp->GetMovementSpeedMultiplier();
        GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed * SpeedMultiplier;
    }
}
```

### Custom Status Effect

```cpp
void AMyCharacter::ApplyCustomToxicCloud()
{
    FStatusEffect ToxicCloud;
    ToxicCloud.EffectType = EStatusEffectType::Poison;
    ToxicCloud.Strength = 2.0f;
    ToxicCloud.Duration = 15.0f;
    ToxicCloud.DamagePerTick = 12.0f;
    ToxicCloud.TickInterval = 0.5f;
    ToxicCloud.bCanStack = true;
    ToxicCloud.MaxStacks = 5;
    ToxicCloud.bIsContagious = true;
    ToxicCloud.ContagionRadius = 500.0f;
    ToxicCloud.ContagionChance = 0.6f;
    ToxicCloud.bCanBeCleansed = false; // Can't be cleansed!

    // Set visuals
    ToxicCloud.Visuals.NiagaraEffect = LoadObject<UNiagaraSystem>(nullptr, TEXT("/Game/VFX/NS_ToxicCloud"));
    ToxicCloud.Visuals.LoopSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/SFX_Toxic_Loop"));
    ToxicCloud.Visuals.EffectColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);

    StatusEffectComp->ApplyCustomStatusEffect(ToxicCloud);
}
```

---

## Advanced Features

### Bleeding Gets Worse With Movement

The **Bleeding** effect automatically scales damage based on movement speed:

```cpp
// In StatusEffectComponent::ApplyDoTDamage
if (Effect.EffectType == EStatusEffectType::Bleeding)
{
    float Speed = Character->GetVelocity().Size();
    float SpeedRatio = Speed / Character->GetMaxSpeed();
    float MovementMultiplier = 1.0f + SpeedRatio; // 1.0x to 2.0x damage
    TotalDamage *= MovementMultiplier;
}
```

Standing still = normal bleeding damage
Running at max speed = double bleeding damage

### Irradiated Prevents Healing

**Irradiated** sets healing multiplier to 0:

```cpp
float HealingMultiplier = StatusEffectComp->GetHealingMultiplier();
float ActualHealing = BaseHealing * HealingMultiplier;
// If irradiated, multiplier = 0, so no healing!
```

### Electrified Chaining

**Electrified** has a chance to become contagious and spread:

```cpp
// 20% chance per second to activate chaining
if (FMath::FRand() < 0.2f * DeltaTime)
{
    Effect.bIsContagious = true;
    Effect.ContagionRadius = 400.0f;
    Effect.ContagionChance = 0.5f;
}
```

### Effect Stacking

Effects can stack for increased potency:

```
Poison (1 stack): 5 damage/tick
Poison (2 stacks): 10 damage/tick
Poison (3 stacks, max): 15 damage/tick
```

```cpp
FStatusEffect Effect = StatusEffectComp->GetStatusEffect(EStatusEffectType::Poison);
int32 Stacks = Effect.CurrentStacks; // How many stacks?
int32 MaxStacks = Effect.MaxStacks;   // Maximum allowed
```

### Configuration

All default values are configurable per instance:

```cpp
// In Details panel or C++
StatusEffectComp->DefaultPoisonDamage = 10.0f;  // Increase poison damage
StatusEffectComp->DefaultSlowPercentage = 0.7f; // Increase slow strength
StatusEffectComp->bEnableCombos = false;        // Disable combos
StatusEffectComp->bEnableContagion = false;     // Disable spreading
StatusEffectComp->bEnableVisuals = true;        // Enable particles/sounds
StatusEffectComp->bShowDebug = true;            // Enable debug logs
```

---

## Multiplayer Considerations

### Replication

The StatusEffectComponent does NOT automatically replicate. You should:

1. **Apply effects on the server**:
```cpp
if (HasAuthority())
{
    StatusEffectComp->ApplyStatusEffect(EStatusEffectType::Fire, 1.0f, 10.0f, this);
}
```

2. **Replicate visual feedback via events**:
```cpp
UFUNCTION(NetMulticast, Reliable)
void Multicast_ShowFireEffect();

// Call this when fire is applied
```

3. **Use Replicated Properties** for important state:
```cpp
UPROPERTY(Replicated)
bool bIsOnFire;

UPROPERTY(Replicated)
bool bIsStunned;
```

---

## Performance Optimization

### Tick Interval

The component ticks 10 times per second (0.1s interval) by default:

```cpp
PrimaryComponentTick.TickInterval = 0.1f;
```

Adjust for your needs:
- **0.05f**: 20 ticks/second (more responsive, higher CPU)
- **0.2f**: 5 ticks/second (less responsive, lower CPU)

### Disable Unused Features

```cpp
StatusEffectComp->bEnableCombos = false;     // No combo checks
StatusEffectComp->bEnableContagion = false;  // No spreading checks
StatusEffectComp->bEnableVisuals = false;    // No particle spawning
```

### Limit Active Effects

Consider capping the number of active effects:

```cpp
if (StatusEffectComp->GetAllActiveEffects().Num() >= 10)
{
    // Don't apply more effects
    return;
}
```

---

## Debugging

Enable debug logging:

```cpp
StatusEffectComp->bShowDebug = true;
```

You'll see logs like:
```
StatusEffect: Applied 0 effect to BP_Zombie_C_1 (Duration: 10.0, Strength: 1.0, Severity: 1)
StatusEffect: Applied 5.0 0 damage to BP_Zombie_C_1
StatusEffect: Triggered combo 0 + 1 = 0 on BP_Zombie_C_1
StatusEffect: Spread 6 effect from BP_Zombie_C_1 to BP_Zombie_C_2
StatusEffect: Cleansed 0 effect from BP_Zombie_C_1
StatusEffect: 0 effect expired on BP_Zombie_C_1
```

---

## Example Use Cases

### 1. Toxic Spitter Zombie

```cpp
// Spitter shoots toxic projectile
Projectile->OnHit.AddDynamic(this, &ASpitter::OnProjectileHit);

void ASpitter::OnProjectileHit(AActor* HitActor)
{
    UStatusEffectComponent* TargetEffects = HitActor->FindComponentByClass<UStatusEffectComponent>();
    if (TargetEffects)
    {
        // Apply poison
        TargetEffects->ApplyStatusEffect(EStatusEffectType::Poison, 1.0f, 8.0f, this);

        // Apply weakness
        TargetEffects->ApplyStatusEffect(EStatusEffectType::Weakness, 1.0f, 12.0f, this);

        // Combo: Poison + Weakness = Diseased (spreads to team!)
    }
}
```

### 2. Fire Barrel Explosion

```cpp
void AFireBarrel::Explode()
{
    TArray<AActor*> NearbyActors;
    // Get actors in radius...

    for (AActor* Actor : NearbyActors)
    {
        UStatusEffectComponent* Effects = Actor->FindComponentByClass<UStatusEffectComponent>();
        if (Effects)
        {
            // Set everyone on fire
            Effects->ApplyStatusEffect(EStatusEffectType::Fire, 1.5f, 15.0f, this);
            // Fire spreads to others nearby!
        }
    }
}
```

### 3. Medic Healing Station

```cpp
void AMedicStation::HealNearbyPlayers(float DeltaTime)
{
    for (APlayerCharacter* Player : PlayersInRange)
    {
        UStatusEffectComponent* Effects = Player->FindComponentByClass<UStatusEffectComponent>();
        if (Effects)
        {
            // Cleanse all debuffs
            Effects->CleanseAllDebuffs();

            // Apply regeneration
            Effects->ApplyStatusEffect(EStatusEffectType::Regeneration, 2.0f, 5.0f, nullptr);

            // Apply damage boost
            Effects->ApplyStatusEffect(EStatusEffectType::DamageBoost, 1.0f, 10.0f, nullptr);
        }
    }
}
```

### 4. Electric Trap

```cpp
void AElectricTrap::OnTrigger(AActor* Victim)
{
    UStatusEffectComponent* Effects = Victim->FindComponentByClass<UStatusEffectComponent>();
    if (Effects)
    {
        // Apply electrified
        Effects->ApplyStatusEffect(EStatusEffectType::Electrified, 1.0f, 5.0f, this);

        // If victim is slowed (water puddle), trigger chain lightning combo!
        if (Effects->HasStatusEffect(EStatusEffectType::Slowing))
        {
            // COMBO: Electrified + Slowing = Chain Lightning + Stun
        }
    }
}
```

### 5. Armor System

```cpp
void AArmor::OnEquipped(APlayerCharacter* Player)
{
    UStatusEffectComponent* Effects = Player->FindComponentByClass<UStatusEffectComponent>();
    if (Effects)
    {
        // Heavy armor provides resistances
        Effects->AddResistance(EStatusEffectType::Fire, 0.5f);      // 50% fire resist
        Effects->AddResistance(EStatusEffectType::Acid, 0.7f);      // 70% acid resist
        Effects->AddResistance(EStatusEffectType::Bleeding, 0.8f);  // 80% bleed resist

        // Apply fortified buff
        Effects->ApplyStatusEffect(EStatusEffectType::Fortified, 1.0f, 99999.0f, nullptr);
    }
}
```

---

## Conclusion

The **StatusEffectComponent** provides a robust, flexible, and scalable system for all your status effect needs. From simple poison damage to complex combo systems and contagious plagues, this component handles it all with clean Blueprint and C++ APIs.

**Key Takeaways:**
- 28 built-in effect types covering all common game mechanics
- Full visual and audio feedback support
- Resistance, immunity, and cleansing mechanics
- Combo system for effect synergies
- Contagion for spreading effects
- Shield system for damage absorption
- Blueprint-friendly with event system
- Fully configurable and extensible

Start simple, expand as needed. Happy game developing!
