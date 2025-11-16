# Zombie Variant Framework - Complete Guide

## 🎯 Overview

This framework provides a **modular, data-driven system** for creating and managing zombie variants in Unreal Engine. It solves the problem of code duplication and maintainability when adding multiple zombie types with different abilities.

### The Problem We're Solving

Without a framework, adding zombie variants leads to:
- ❌ Duplicated code across zombie types
- ❌ Difficult to maintain and extend
- ❌ Hard to balance and configure
- ❌ Messy inheritance hierarchies
- ❌ Multiplayer replication nightmares

### Our Solution

✅ **Single base zombie class** (AZombieBase)
✅ **Modular ability system** (UZombieAbilityComponent)
✅ **Data-driven configuration** (Data Tables)
✅ **Easy variant creation** (just add abilities!)
✅ **Built-in multiplayer support**
✅ **Clean, maintainable architecture**

---

## 📐 Architecture Overview

```
┌────────────────────────────────────────────────────────────┐
│                     ZOMBIE FRAMEWORK                        │
└────────────────────────────────────────────────────────────┘

┌─────────────────┐      Uses       ┌──────────────────────┐
│                 │ ──────────────► │   Data Table         │
│   AZombieBase   │                 │   (Config Rows)      │
│  (Base Class)   │                 └──────────────────────┘
│                 │                          │
└────────┬────────┘                          │
         │ Has Many                          │ Defines
         │                                   ▼
         │                          ┌──────────────────────┐
         │                          │ FZombieConfigData    │
         │                          │ - Health, Speed, etc │
         │                          │ - Ability Classes    │
         │                          └──────────────────────┘
         │
         ▼
┌────────────────────────────────────────────────────────────┐
│         UZombieAbilityComponent (Base Ability)             │
│  - Lifecycle (Init, Activate, Update, Deactivate)          │
│  - Targeting                                                │
│  - Events (OnDamaged, OnTargetDetected, etc.)              │
│  - Tags & Priority                                          │
└────────────────────────────────────────────────────────────┘
                              │
         ┌────────────────────┼────────────────────┐
         │                    │                    │
         ▼                    ▼                    ▼
  ┌─────────────┐     ┌──────────────┐     ┌─────────────┐
  │  Climbing   │     │   Spitter    │     │  Tunneler   │
  │   Ability   │     │   Ability    │     │   Ability   │
  └─────────────┘     └──────────────┘     └─────────────┘
```

---

## 🏗️ Core Components

### 1. AZombieBase - The Foundation

**Location:** `Source/cleanzombie/Public/ZombieBase.h`

**Purpose:** Single base class for all zombies, regardless of variant type.

**Key Features:**
- Health and stat management
- Ability component system
- Configuration loading from data tables
- Target management
- Event system for abilities to hook into
- Full network replication

**Usage:**
```cpp
// In Blueprint or C++
AZombieBase* Zombie = GetWorld()->SpawnActor<AZombieBase>();
Zombie->InitializeFromVariant(EZombieVariant::Climber);
```

### 2. UZombieAbilityComponent - The Building Blocks

**Location:** `Source/cleanzombie/Public/ZombieAbilityComponent.h`

**Purpose:** Base class for all zombie abilities. Inherit from this to create new abilities.

**Lifecycle:**
```
InitializeAbility()
     ↓
CanActivate()? → No → Wait
     ↓ Yes
ActivateAbility()
     ↓
UpdateAbility(DeltaTime) [Every frame while active]
     ↓
DeactivateAbility()
```

**Key Features:**
- Automatic lifecycle management
- Priority system for ability conflicts
- Tag-based blocking (e.g., can't climb while stunned)
- Event callbacks for zombie actions
- Built-in debug visualization

### 3. FZombieConfigData - The Configuration

**Location:** `Source/cleanzombie/Public/ZombieBase.h` (struct definition)

**Purpose:** Data structure defining a zombie variant's stats and abilities.

**Fields:**
```cpp
struct FZombieConfigData
{
    EZombieVariant VariantType;         // Type enum
    FText DisplayName;                   // "Wall Climber"
    float MaxHealth;                     // 100.0
    float MovementSpeed;                 // 300.0
    float AttackDamage;                  // 20.0
    float AttackRange;                   // 150.0
    float AttackRate;                    // 1.0
    float DetectionRange;                // 1500.0
    TArray<TSubclassOf<UZombieAbilityComponent>> AbilityClasses;
    USkeletalMesh* ZombieMesh;          // Visual mesh
    TSubclassOf<UAnimInstance> AnimationBlueprint;
    FVector ScaleMultiplier;             // (1.0, 1.0, 1.0)
    int32 PointValue;                    // 50
    float SpawnWeight;                   // 1.0
};
```

---

## 🎮 Creating Zombie Variants

### Method 1: Using Data Tables (Recommended)

#### Step 1: Create Data Table Asset

1. **In Unreal Editor:**
   - Right-click in Content Browser
   - **Miscellaneous** → **Data Table**
   - Choose Row Structure: `ZombieConfigData`
   - Name it: `DT_ZombieVariants`

#### Step 2: Add Variant Rows

| Row Name | Variant Type | Health | Speed | Abilities |
|----------|--------------|--------|-------|-----------|
| `Basic` | Basic | 100 | 300 | (none) |
| `Climber` | Climber | 120 | 200 | ClimbingAbility |
| `Spitter` | Spitter | 80 | 250 | SpitterAbility |
| `Tank` | Tank | 300 | 150 | (none) |
| `Runner` | Runner | 60 | 600 | (none) |

#### Step 3: Assign to Zombie Blueprint

```blueprint
BaseZombie Blueprint:
├─► Default Values
│   ├─► Zombie Config Table: DT_ZombieVariants
│   └─► Config Row Name: "Climber"
└─► Event Begin Play
    └─► Initialize From Config (Row Name)
```

### Method 2: Programmatic Creation

```cpp
// In C++ or Blueprint
AZombieBase* Zombie = GetWorld()->SpawnActor<AZombieBase>();

// Add abilities manually
Zombie->AddAbility(UClimbingAbility::StaticClass());
Zombie->AddAbility(USpitterAbility::StaticClass());

// Set stats
Zombie->MaxHealth = 150.0f;
Zombie->CurrentHealth = 150.0f;
```

### Method 3: Blueprint Child Classes

1. **Create Blueprint** child of `ZombieBase`
2. **Name it:** `BP_ClimberZombie`
3. **Add Components:**
   - Climbing Ability Component
4. **Set Default Values:**
   - Max Health: 120
   - Movement Speed: 200
5. **Configure Ability:**
   - Climbing Ability → Auto Climbing: ☑
   - Climbing Ability → Drop To Attack: ☑

---

## 🔧 Creating New Abilities

### Step-by-Step: Creating a "Tunneler" Ability

#### 1. Create Header File

```cpp
// TunnelerAbility.h
#pragma once

#include "CoreMinimal.h"
#include "ZombieAbilityComponent.h"
#include "TunnelerAbility.generated.h"

UCLASS()
class CLEANZOMBIE_API UTunnelerAbility : public UZombieAbilityComponent
{
    GENERATED_BODY()

public:
    UTunnelerAbility();

    // Override lifecycle
    virtual void InitializeAbility_Implementation() override;
    virtual bool ActivateAbility_Implementation() override;
    virtual void DeactivateAbility_Implementation() override;
    virtual void UpdateAbility_Implementation(float DeltaTime) override;
    virtual bool CanActivate_Implementation() const override;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunneler")
    float TunnelSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tunneler")
    float MaxTunnelDistance = 1000.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Tunneler")
    void StartTunneling();

    UFUNCTION(BlueprintCallable, Category = "Tunneler")
    void StopTunneling();

protected:
    bool bIsTunneling = false;
    FVector TunnelStartLocation;
    FVector TunnelTargetLocation;
};
```

#### 2. Implement Functionality

```cpp
// TunnelerAbility.cpp
#include "TunnelerAbility.h"
#include "GameFramework/Character.h"

UTunnelerAbility::UTunnelerAbility()
{
    AbilityName = FText::FromString(TEXT("Tunnel"));
    AbilityDescription = FText::FromString(TEXT("Burrow underground"));
    AbilityPriority = 75; // Higher priority
    bCanRunConcurrently = false;

    // Tags
    AbilityTags.Add(TEXT("Tunneling"));
    AbilityTags.Add(TEXT("Moving"));
    BlockingTags.Add(TEXT("Climbing"));
    BlockingTags.Add(TEXT("Stunned"));
}

void UTunnelerAbility::InitializeAbility_Implementation()
{
    Super::InitializeAbility_Implementation();
    // Setup code
}

bool UTunnelerAbility::ActivateAbility_Implementation()
{
    if (!Super::ActivateAbility_Implementation())
    {
        return false;
    }

    StartTunneling();
    return true;
}

void UTunnelerAbility::DeactivateAbility_Implementation()
{
    StopTunneling();
    Super::DeactivateAbility_Implementation();
}

void UTunnelerAbility::UpdateAbility_Implementation(float DeltaTime)
{
    Super::UpdateAbility_Implementation(DeltaTime);

    if (bIsTunneling && OwnerZombie)
    {
        // Move underground toward target
        FVector CurrentLocation = OwnerZombie->GetActorLocation();
        FVector Direction = (TunnelTargetLocation - CurrentLocation).GetSafeNormal();

        FVector NewLocation = CurrentLocation + (Direction * TunnelSpeed * DeltaTime);
        OwnerZombie->SetActorLocation(NewLocation);

        // Check if reached destination
        if (FVector::Dist(CurrentLocation, TunnelTargetLocation) < 50.0f)
        {
            DeactivateAbility();
        }
    }
}

bool UTunnelerAbility::CanActivate_Implementation() const
{
    if (!Super::CanActivate_Implementation())
    {
        return false;
    }

    // Can only tunnel if on ground
    if (OwnerZombie && OwnerZombie->GetCharacterMovement())
    {
        return OwnerZombie->GetCharacterMovement()->IsMovingOnGround();
    }

    return false;
}

void UTunnelerAbility::StartTunneling()
{
    if (!OwnerZombie)
    {
        return;
    }

    bIsTunneling = true;
    TunnelStartLocation = OwnerZombie->GetActorLocation();

    // Calculate tunnel target (toward current target or forward)
    if (HasValidTarget())
    {
        TunnelTargetLocation = CurrentTarget->GetActorLocation();
    }
    else
    {
        TunnelTargetLocation = TunnelStartLocation +
            (OwnerZombie->GetActorForwardVector() * MaxTunnelDistance);
    }

    // Hide zombie (visual effect)
    OwnerZombie->SetActorHiddenInGame(true);
    OwnerZombie->SetActorEnableCollision(false);

    UE_LOG(LogTemp, Log, TEXT("Tunneler: Started tunneling!"));
}

void UTunnelerAbility::StopTunneling()
{
    if (!OwnerZombie)
    {
        return;
    }

    bIsTunneling = false;

    // Show zombie again
    OwnerZombie->SetActorHiddenInGame(false);
    OwnerZombie->SetActorEnableCollision(true);

    UE_LOG(LogTemp, Log, TEXT("Tunneler: Emerged from tunnel!"));
}
```

#### 3. Add to Variant Configuration

In your Data Table:
```
Row: "Tunneler"
├─► Variant Type: Custom
├─► Display Name: "Tunneling Zombie"
├─► Max Health: 90
├─► Movement Speed: 350
├─► Ability Classes: [TunnelerAbility]
└─► Point Value: 75
```

#### 4. Use in Game

```blueprint
Spawn Zombies:
└─► For Each Variant
    ├─► Spawn Actor (ZombieBase)
    └─► Initialize From Config (VariantName)
```

---

## 📊 Example Zombie Variants

### Climber Zombie

```cpp
Config:
├─► Health: 120
├─► Speed: 200
├─► Abilities: [ClimbingAbility]
└─► Strategy: Flanks via walls/ceilings, drop attacks

Strengths:
✓ Reaches high places
✓ Ambush from above
✓ Bypasses ground obstacles

Weaknesses:
✗ Slower movement
✗ Vulnerable while climbing
```

### Spitter Zombie

```cpp
Config:
├─► Health: 80
├─► Speed: 250
├─► Abilities: [SpitterAbility]
└─► Strategy: Ranged harassment, keeps distance

Strengths:
✓ Attacks from safety
✓ Zones control
✓ Supports other zombies

Weaknesses:
✗ Low health
✗ Weak in melee
```

### Tank Zombie

```cpp
Config:
├─► Health: 300
├─► Speed: 150
├─► Abilities: []
└─► Strategy: Soaks damage, blocks paths

Strengths:
✓ High health
✓ Resistant to knockback
✓ Intimidating presence

Weaknesses:
✗ Very slow
✗ Easy to kite
```

### Runner Zombie

```cpp
Config:
├─► Health: 60
├─► Speed: 600
├─► Abilities: []
└─► Strategy: Fast pursuit, overwhelm with speed

Strengths:
✓ Catches fleeing players
✓ Closes distance quickly
✓ Hard to hit

Weaknesses:
✗ Low health
✗ Dies easily
```

### Hybrid: Climber-Spitter

```cpp
Config:
├─► Health: 100
├─► Speed: 220
├─► Abilities: [ClimbingAbility, SpitterAbility]
└─► Strategy: Climbs to vantage points, spits from above

Strengths:
✓ Versatile threat
✓ Vertical advantage
✓ Safe ranged attacks

Weaknesses:
✗ Moderate in all stats
✗ No specialization
```

---

## 🎯 Ability System Deep Dive

### Ability Lifecycle Example

```cpp
// Player enters detection range
ZombieBase->OnTargetDetected(Player);
    ↓
// All abilities are notified
ClimbingAbility->OnZombieDetectedTarget(Player);
SpitterAbility->OnZombieDetectedTarget(Player);
    ↓
// Abilities evaluate if they should activate
ClimbingAbility->CanActivate() → Checks if target is high up
SpitterAbility->CanActivate() → Checks if target in range
    ↓
// Higher priority ability activates first
ClimbingAbility->ActivateAbility()
    ├─► Adds "Climbing" tag
    ├─► Blocks SpitterAbility (has "Climbing" in BlockingTags)
    └─► Starts climbing movement
    ↓
// Update every frame
ClimbingAbility->UpdateAbility(DeltaTime)
    ├─► Moves zombie up wall
    ├─► Checks if should drop attack
    └─► Updates climbing state
    ↓
// Conditions met for ability to end
ClimbingAbility->DeactivateAbility()
    ├─► Removes "Climbing" tag
    ├─► Stops climbing movement
    └─► Spitter is now un-blocked
    ↓
// Spitter can now activate
SpitterAbility->ActivateAbility()
    └─► Fires projectile at player
```

### Ability Priority System

Abilities with **higher priority** activate first when multiple abilities want to activate:

```cpp
Priority Levels (examples):
├─► 100: Emergency (e.g., Explode when low health)
├─►  75: Special Movement (e.g., Tunneling, Leaping)
├─►  50: Combat (e.g., Spitting, Exploding)
├─►  25: Movement (e.g., Climbing, Running)
└─►   0: Passive (e.g., Regeneration, Aura)
```

### Ability Tags

**Tags prevent conflicts between abilities:**

```cpp
Example: Climbing Ability
├─► Ability Tags (added when active):
│   ├─► "Climbing"
│   └─► "Moving"
├─► Blocking Tags (prevent activation if present):
│   ├─► "Attacking"
│   ├─► "Stunned"
│   └─► "Tunneling"

Example: Spitter Ability
├─► Ability Tags:
│   ├─► "Attacking"
│   └─► "Ranged"
├─► Blocking Tags:
│   ├─► "Climbing" ← Can't spit while climbing
│   ├─► "Stunned"
│   └─► "Tunneling"
```

---

## 🕹️ Integration with Existing Systems

### Integrating with Your Current Zombies

If you have existing zombie Blueprints (`BaseZombie`, `ParentEnemyBP`):

#### Option 1: Reparent to ZombieBase

1. Open your existing zombie Blueprint
2. **File** → **Reparent Blueprint**
3. Choose **ZombieBase** as new parent
4. Fix any broken references
5. Add ability components

#### Option 2: Migration Path

```blueprint
Migration Steps:
1. Create new ZombieBase Blueprint: BP_ZombieBase
2. Copy visual mesh & animations from BaseZombie
3. Add ability components
4. Test new zombie alongside old zombies
5. Gradually replace old zombies in spawners
6. Deprecate old zombie classes
```

### Integrating with Wave/Spawn System

```cpp
// In your ZombieSpawnerZone or similar
void AZombieSpawner::SpawnZombieWave(int32 WaveNumber)
{
    // Determine variant based on wave
    FName VariantRow = SelectVariantForWave(WaveNumber);

    // Spawn zombie
    AZombieBase* Zombie = GetWorld()->SpawnActor<AZombieBase>(
        ZombieBaseClass,
        SpawnLocation,
        SpawnRotation
    );

    // Initialize from config
    if (Zombie)
    {
        Zombie->InitializeFromConfig(VariantRow);
        Zombie->SetTarget(FindNearestPlayer());
    }
}

FName AZombieSpawner::SelectVariantForWave(int32 WaveNumber)
{
    // Early waves: Basic zombies
    if (WaveNumber < 5)
    {
        return TEXT("Basic");
    }
    // Mid waves: Mix of variants
    else if (WaveNumber < 10)
    {
        TArray<FName> Variants = {TEXT("Basic"), TEXT("Runner"), TEXT("Climber")};
        return Variants[FMath::RandRange(0, Variants.Num() - 1)];
    }
    // Late waves: Advanced variants
    else
    {
        TArray<FName> Variants = {TEXT("Climber"), TEXT("Spitter"), TEXT("Tank")};
        return Variants[FMath::RandRange(0, Variants.Num() - 1)];
    }
}
```

---

## 🛠️ Debug & Testing Tools

### Visual Debugging

Enable debug visualization:

```cpp
// On ZombieBase
Zombie->bShowDebugInfo = true;

// On individual abilities
ClimbingAbility->bShowDebug = true;
```

**What you'll see:**
- Zombie stats (health, active abilities)
- Target lines
- Ability-specific debug info
- Surface normals (climbing)
- Projectile trajectories (spitter)

### Console Commands (Create These)

```cpp
// Example console commands to add

// Spawn specific variant
UFUNCTION(Exec)
void SpawnZombieVariant(FName VariantName)
{
    AZombieBase* Zombie = GetWorld()->SpawnActor<AZombieBase>(
        ZombieBaseClass,
        PlayerLocation + PlayerForward * 300,
        FRotator::ZeroRotator
    );
    Zombie->InitializeFromConfig(VariantName);
}

// Toggle debug for all zombies
UFUNCTION(Exec)
void DebugZombies(bool bEnable)
{
    TArray<AActor*> Zombies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieBase::StaticClass(), Zombies);

    for (AActor* Actor : Zombies)
    {
        if (AZombieBase* Zombie = Cast<AZombieBase>(Actor))
        {
            Zombie->bShowDebugInfo = bEnable;
        }
    }
}

// List all active abilities
UFUNCTION(Exec)
void ListZombieAbilities()
{
    TArray<AActor*> Zombies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AZombieBase::StaticClass(), Zombies);

    for (AActor* Actor : Zombies)
    {
        if (AZombieBase* Zombie = Cast<AZombieBase>(Actor))
        {
            for (UZombieAbilityComponent* Ability : Zombie->GetActiveAbilities())
            {
                UE_LOG(LogTemp, Log, TEXT("%s: %s (%s)"),
                    *Zombie->GetName(),
                    *Ability->AbilityName.ToString(),
                    Ability->bIsActive ? TEXT("ACTIVE") : TEXT("Inactive")
                );
            }
        }
    }
}
```

---

## 📈 Performance Considerations

### Ability Ticking

By default, abilities tick every frame. Optimize by:

```cpp
// Reduce tick frequency for non-critical abilities
PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS instead of 60

// Or disable ticking entirely for event-driven abilities
PrimaryComponentTick.bCanEverTick = false;
```

### Ability Component Pooling

For frequently added/removed abilities:

```cpp
// Instead of destroying, deactivate and reuse
Zombie->DeactivateAbility(AbilityClass);
// Ability remains on zombie, just inactive

// Vs. removing (more expensive)
Zombie->RemoveAbility(Ability);
```

### Data Table Caching

Cache frequently accessed config rows:

```cpp
// In game instance or singleton
TMap<FName, FZombieConfigData> CachedConfigs;

FZombieConfigData GetCachedConfig(FName RowName)
{
    if (CachedConfigs.Contains(RowName))
    {
        return CachedConfigs[RowName];
    }

    // Load from table and cache
    FZombieConfigData* Config = ConfigTable->FindRow<FZombieConfigData>(RowName, TEXT(""));
    if (Config)
    {
        CachedConfigs.Add(RowName, *Config);
        return *Config;
    }

    return FZombieConfigData();
}
```

---

## 🔄 Multiplayer Replication

### Automatic Replication

The framework handles replication automatically:

- ✅ `AZombieBase` stats (health, target) replicated
- ✅ Ability activation replicated via tags
- ✅ Component state synchronized

### Custom Ability Replication

If your ability needs custom replication:

```cpp
// In your ability header
UPROPERTY(Replicated)
bool bCustomState;

// In ability .cpp
void UYourAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UYourAbility, bCustomState);
}
```

### Server Authority

Abilities should only activate on server for AI zombies:

```cpp
bool UYourAbility::ActivateAbility_Implementation()
{
    // Only activate on server for AI
    if (GetOwner()->GetLocalRole() != ROLE_Authority)
    {
        return false;
    }

    return Super::ActivateAbility_Implementation();
}
```

---

## 📝 Quick Reference

### Adding a New Zombie Variant

1. ✅ Create ability component(s) if needed
2. ✅ Add row to `DT_ZombieVariants` data table
3. ✅ Configure stats and ability classes
4. ✅ Test with `SpawnZombieVariant` console command
5. ✅ Add to wave spawner logic

### Adding a New Ability

1. ✅ Create header file (inherit from UZombieAbilityComponent)
2. ✅ Implement lifecycle functions
3. ✅ Add configuration properties
4. ✅ Compile and test in editor
5. ✅ Add to zombie variant configs

### Modifying Existing Zombies

1. ✅ Find zombie in spawner
2. ✅ Change `ConfigRowName` to desired variant
3. ✅ Or call `InitializeFromConfig(NewRow)` at runtime

---

## 🎓 Best Practices

### Do's ✅

- **Use data tables** for variant configuration
- **Keep abilities focused** (one ability = one behavior)
- **Use ability tags** to prevent conflicts
- **Cache references** in `InitializeAbility()`
- **Profile performance** with many zombies
- **Test in multiplayer** early and often

### Don'ts ❌

- **Don't put gameplay logic in ZombieBase** - use abilities!
- **Don't create deep inheritance** - use composition (abilities)
- **Don't forget replication** - test server/client
- **Don't activate abilities from client** - server authoritative
- **Don't tick unnecessarily** - use events when possible

---

## 🚀 Next Steps

1. **Compile** the framework code
2. **Create** `DT_ZombieVariants` data table
3. **Add** basic zombie variants (Basic, Climber, Runner)
4. **Test** spawning different variants
5. **Create** custom abilities for your game
6. **Integrate** with existing zombie spawners
7. **Balance** stats and abilities
8. **Polish** with animations and effects

---

## 📚 Related Documentation

- [CLIMBING_SYSTEM_GUIDE.md](CLIMBING_SYSTEM_GUIDE.md) - Climbing ability details
- [ANIMATION_SETUP_GUIDE.md](ANIMATION_SETUP_GUIDE.md) - Animation integration
- [CLIMBING_SYSTEM_README.md](CLIMBING_SYSTEM_README.md) - Climbing system overview

---

**You now have a complete, modular, data-driven zombie variant framework!** 🧟‍♂️🎮

Add new zombie types by:
1. Creating an ability component
2. Adding a data table row
3. Done!

No more messy inheritance or code duplication. Just clean, maintainable, expandable zombie variants.

