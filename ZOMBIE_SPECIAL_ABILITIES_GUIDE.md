# Zombie Special Abilities Guide

## Overview

This guide covers **6 specialized zombie abilities** for creating diverse and challenging enemy types in your Gears of War style shooter. Each ability is fully modular, networkable, and customizable.

---

## Table of Contents

1. [Leaper Zombie](#1-leaper-zombie)
2. [Exploder Zombie](#2-exploder-zombie)
3. [Tank Zombie](#3-tank-zombie)
4. [Screamer Zombie](#4-screamer-zombie)
5. [Burrower Zombie](#5-burrower-zombie)
6. [Summoner Zombie](#6-summoner-zombie)
7. [Quick Setup Guide](#quick-setup-guide)
8. [Combining Abilities](#combining-abilities)

---

## 1. Leaper Zombie

**Pounces on targets from long distances and pins them down**

### Core Mechanics
- Detects targets in range automatically
- Leaps toward target with arc trajectory
- Pins target on landing (disables movement)
- Deals damage while pinning
- Target can break free by applying force

### Configuration

| Property | Default | Description |
|----------|---------|-------------|
| MaxLeapRange | 1500 | Maximum leap distance |
| MinLeapRange | 300 | Minimum leap distance |
| LeapSpeed | 2000 | Speed during leap |
| LeapArcHeight | 0.5 | Arc height multiplier |
| ImpactDamage | 50 | Damage on landing |
| PinDamagePerSecond | 20 | DoT while pinning |
| MaxPinDuration | 5 | Maximum pin time |
| BreakFreeThreshold | 100 | Force needed to escape |
| LeapCooldown | 8 | Cooldown between leaps |
| StunDuration | 1 | Stun time on impact |

### Usage Example

**Create Blueprint:**
1. Create new Blueprint based on `ZombieBase`
2. Add `LeaperAbility` component
3. Configure leap parameters
4. Override Blueprint events for custom behavior

**C++ Usage:**
```cpp
ULeaperAbility* LeaperAbility = OwnerZombie->FindComponentByClass<ULeaperAbility>();
if (LeaperAbility && LeaperAbility->CanLeap())
{
    LeaperAbility->ExecuteLeap();
}

// Target breaks free
if (LeaperAbility->IsPinning())
{
    LeaperAbility->BreakFree(50.0f); // Apply force to break free
}
```

### Blueprint Events
- **OnLeapStarted** - Called when leap begins
- **OnLeapImpact** - Called on landing
- **OnPinStarted** - Called when pinning starts
- **OnPinEnded** - Called when pin ends
- **OnLeapMissed** - Called if leap misses

### Tactical Uses
- **Ambush**: Hide and leap on unaware players
- **Flanking**: Jump over cover to reach backline
- **Crowd Control**: Pin high-value targets
- **Verticality**: Leap to elevated positions

---

## 2. Exploder Zombie

**Suicide bomber that detonates near targets**

### Core Mechanics
- Auto-detects nearby targets
- Charges (wind-up) before detonating
- Massive AoE damage with falloff
- Applies status effects in blast radius
- Can detonate on death
- Multiple explosion types

### Explosion Types

| Type | Effect |
|------|--------|
| **Standard** | Normal explosion |
| **Fire** | Sets targets on fire |
| **Acid** | Applies acid DoT |
| **Poison** | Creates poison cloud + diseased |
| **Shrapnel** | Causes bleeding |
| **EMP** | Stuns + disarms |
| **Nuclear** | Massive damage + irradiation |

### Configuration

| Property | Default | Description |
|----------|---------|-------------|
| ExplosionDamage | 200 | Base explosion damage |
| ExplosionRadius | 800 | Blast radius |
| DetonationProximity | 300 | Auto-detonate distance |
| ChargeTime | 2 | Wind-up time |
| bDetonateOnDeath | true | Explode when killed |
| bSuicideExplosion | true | Kills self |
| DamageFalloff | 0.7 | Damage reduction by distance |
| ChargingSpeedMultiplier | 1.5 | Speed boost while charging |

### Usage Example

**Create Exploder:**
```cpp
UExploderAbility* Exploder = CreateDefaultSubobject<UExploderAbility>(TEXT("ExploderAbility"));
Exploder->ExplosionType = EExplosionType::Fire;
Exploder->ExplosionDamage = 250.0f;
Exploder->ExplosionRadius = 1000.0f;

// Manually trigger
Exploder->Detonate();

// Start charging
Exploder->StartCharging();

// Cancel charge
Exploder->CancelCharge();
```

### Blueprint Events
- **OnChargingStarted** - Wind-up begins
- **OnExplosionTriggered** - Detonation occurs
- **OnActorDamaged** - Actor hit by explosion
- **OnChargingCancelled** - Charge interrupted

### Tactical Uses
- **Area Denial**: Force players out of cover
- **Kamikaze Rush**: Charge into grouped players
- **Combo Detonations**: Multiple exploders chain
- **Death Trap**: Kill to trigger explosion

---

## 3. Tank Zombie

**Heavy charger that smashes through obstacles**

### Core Mechanics
- Charges toward targets in straight line
- Breaks through barricades and destructibles
- Knocks back and stuns hit targets
- High damage, difficult to stop
- Short preparation phase before charge

### Configuration

| Property | Default | Description |
|----------|---------|-------------|
| ChargeSpeed | 1500 | Speed during charge |
| ChargeDuration | 3 | How long charge lasts |
| ChargeDamage | 100 | Damage to targets |
| KnockbackForce | 2000 | Knockback strength |
| ChargeCooldown | 10 | Cooldown between charges |
| PreparationTime | 0.5 | Wind-up before charge |
| bCanBreakObstacles | true | Destroys obstacles |
| ObstacleDamage | 500 | Damage to destructibles |

### Usage Example

**Create Tank:**
```cpp
UTankAbility* Tank = CreateDefaultSubobject<UTankAbility>(TEXT("TankAbility"));
Tank->ChargeSpeed = 2000.0f;
Tank->ChargeDamage = 150.0f;
Tank->bCanBreakObstacles = true;

// Execute charge
Tank->ExecuteCharge();

// Check if charging
if (Tank->IsCharging())
{
    // Tank is charging!
}
```

### Blueprint Events
- **OnChargeStarted** - Charge begins
- **OnChargeImpact** - Hits target
- **OnObstacleDestroyed** - Breaks through obstacle
- **OnChargeEnded** - Charge completes

### Tactical Uses
- **Breakthrough**: Smash barricades
- **Crowd Dispersal**: Scatter grouped players
- **Chase Down**: Run down fleeing targets
- **Corridor Dominance**: Control narrow passages

---

## 4. Screamer Zombie

**Sonic scream that stuns and disorients**

### Core Mechanics
- Winds up before releasing scream
- AoE stun in large radius
- Causes blindness and confusion
- Deals moderate damage
- Long cooldown for balance

### Configuration

| Property | Default | Description |
|----------|---------|-------------|
| ScreamRadius | 1000 | Effect radius |
| StunDuration | 3 | Stun time |
| ScreamDamage | 25 | Damage dealt |
| ScreamCooldown | 15 | Cooldown |
| bCausesBlinding | true | Applies blind effect |
| bCausesConfusion | true | Applies confusion |
| WindupTime | 1 | Preparation time |

### Usage Example

**Create Screamer:**
```cpp
UScreamerAbility* Screamer = CreateDefaultSubobject<UScreamerAbility>(TEXT("ScreamerAbility"));
Screamer->ScreamRadius = 1200.0f;
Screamer->StunDuration = 4.0f;
Screamer->bCausesBlinding = true;

// Execute scream
Screamer->ExecuteScream();

// Check if can scream
if (Screamer->CanScream())
{
    // Ready to scream
}
```

### Blueprint Events
- **OnScreamStarted** - Wind-up begins
- **OnActorAffected** - Actor hit by scream

### Tactical Uses
- **Team Wipe**: Stun entire squad
- **Support**: Disable players for other zombies
- **Escape Tool**: Stun pursuers
- **Combo Setup**: Set up for other abilities

---

## 5. Burrower Zombie

**Digs underground for surprise attacks**

### Core Mechanics
- Burrows underground (becomes invisible)
- Moves underground toward targets
- Emerges for surprise attack
- Deals damage and knocks up on emergence
- Limited underground time

### Configuration

| Property | Default | Description |
|----------|---------|-------------|
| BurrowSpeed | 800 | Underground movement speed |
| MaxUndergroundTime | 10 | Max time underground |
| EmergeDamage | 75 | Damage on emergence |
| EmergeRadius | 300 | Knock-up radius |
| BurrowCooldown | 12 | Cooldown |
| bInvisibleUnderground | true | Hidden while burrowed |

### Usage Example

**Create Burrower:**
```cpp
UBurrowerAbility* Burrower = CreateDefaultSubobject<UBurrowerAbility>(TEXT("BurrowerAbility"));
Burrower->BurrowSpeed = 1000.0f;
Burrower->EmergeDamage = 100.0f;

// Burrow
Burrower->Burrow();

// Emerge at location
Burrower->Emerge(TargetLocation);

// Check if underground
if (Burrower->IsUnderground())
{
    // Currently burrowed
}
```

### Blueprint Events
- **OnBurrowed** - Goes underground
- **OnEmerged** - Surfaces

### Tactical Uses
- **Ambush**: Surprise attack from below
- **Repositioning**: Escape danger underground
- **Bypassing Defenses**: Tunnel under barricades
- **Psychological**: Create paranoia

---

## 6. Summoner Zombie

**Spawns minion zombies**

### Core Mechanics
- Channels to summon minions
- Spawns multiple minions in circle
- Manages active minion count
- Minions can persist or die with summoner
- Long cooldown for balance

### Configuration

| Property | Default | Description |
|----------|---------|-------------|
| MinionsPerSummon | 3 | Minions spawned |
| MaxActiveMinions | 10 | Maximum alive minions |
| SummonRadius | 500 | Spawn radius around summoner |
| SummonCooldown | 20 | Cooldown |
| SummonChannelTime | 2 | Casting time |
| bMinionsSurviveOnDeath | false | Minions persist after death |

### Usage Example

**Create Summoner:**
```cpp
USummonerAbility* Summoner = CreateDefaultSubobject<USummonerAbility>(TEXT("SummonerAbility"));
Summoner->MinionClass = AMinionZombie::StaticClass();
Summoner->MinionsPerSummon = 5;
Summoner->MaxActiveMinions = 15;

// Summon minions
Summoner->SummonMinions();

// Get active count
int32 MinionCount = Summoner->GetActiveMinionCount();

// Check if can summon
if (Summoner->CanSummon())
{
    // Ready to summon
}
```

### Blueprint Events
- **OnSummonStarted** - Channeling begins
- **OnMinionSpawned** - Minion created

### Tactical Uses
- **Overwhelming**: Flood with numbers
- **Distraction**: Minions draw fire
- **Multi-Front**: Attack from all sides
- **Boss Encounter**: Summoner + minions

---

## Quick Setup Guide

### Adding Ability to Zombie

**In Blueprint:**
1. Open zombie Blueprint
2. Add component → Search for ability (e.g., "Leaper Ability")
3. Configure properties in Details panel
4. Override events for custom behavior
5. Compile and test

**In C++:**
```cpp
// In zombie constructor
ULeaperAbility* LeaperComp = CreateDefaultSubobject<ULeaperAbility>(TEXT("LeaperAbility"));

// Configure
LeaperComp->MaxLeapRange = 2000.0f;
LeaperComp->ImpactDamage = 75.0f;

// Activate
LeaperComp->Activate();
```

### Network Replication

All abilities support multiplayer:
```cpp
// Server-side execution
if (HasAuthority())
{
    LeaperAbility->ExecuteLeap();
}

// Replicate visual effects via Blueprint events
UFUNCTION(NetMulticast, Reliable)
void Multicast_ShowLeapEffect();
```

---

## Combining Abilities

You can add multiple abilities to create hybrid zombies!

### Example Combinations

**1. Leaper-Exploder ("Kamikaze Leaper")**
```cpp
// Leaps on target, pins them, then explodes
ULeaperAbility* Leaper;
UExploderAbility* Exploder;

// On pin started, start explosion charge
void OnPinStarted_Implementation(AActor* Target)
{
    Exploder->StartCharging();
}
```

**2. Tank-Screamer ("Juggernaut")**
```cpp
// Screams to stun, then charges through stunned enemies
UScreamerAbility* Screamer;
UTankAbility* Tank;

void OnScreamStarted_Implementation()
{
    // Delay charge until after scream
    GetWorld()->GetTimerManager().SetTimer(ChargeTimer, this, &ThisClass::ExecuteTankCharge, 1.5f);
}
```

**3. Burrower-Summoner ("Ambush Leader")**
```cpp
// Burrows near players, emerges and summons minions
UBurrowerAbility* Burrower;
USummonerAbility* Summoner;

void OnEmerged_Implementation(FVector Location)
{
    // Summon minions immediately after emerging
    Summoner->SummonMinions();
}
```

---

## Best Practices

### Performance
- Limit active special zombies (5-10 max recommended)
- Use object pooling for summoner minions
- Disable abilities when far from players
- Configure tick intervals appropriately

### Balance
- Test cooldowns extensively
- Adjust damage based on difficulty
- Provide audio/visual warnings (screamer wind-up, exploder charging)
- Give players counterplay options (break free from leaper, dodge tank charge)

### Design
- Use abilities contextually (burrower in open areas, tank in corridors)
- Create variety in encounters
- Telegraph attacks clearly
- Reward skilled play (dodging leap, fleeing explosion)

---

## Files Created

- `LeaperAbility.h/cpp` - Pounce and pin mechanic
- `ExploderAbility.h/cpp` - Suicide bomber with 7 explosion types
- `TankAbility.h/cpp` - Heavy charge ability
- `ScreamerAbility.h/cpp` - AoE stun scream
- `BurrowerAbility.h/cpp` - Underground movement
- `SummonerAbility.h/cpp` - Minion spawning

All abilities inherit from `ZombieAbilityComponent` and integrate with the existing modular system.

---

## Conclusion

These 6 specialized zombie abilities provide incredible variety and depth to your zombie encounters. Mix, match, and combine them to create unique and challenging enemy types that keep players on their toes!

**Next Steps:**
- Create zombie variants using these abilities
- Design encounters around ability synergies
- Balance for your difficulty curve
- Create Blueprint derivatives with custom visuals

Happy zombie hunting! 🧟‍♂️💀
