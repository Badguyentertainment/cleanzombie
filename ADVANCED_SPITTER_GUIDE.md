# Advanced Spit Attack System - Complete Guide

## 🎯 Overview

A comprehensive ranged attack system for zombie variants featuring multiple projectile types, arc trajectories, ground puddles with damage-over-time, status effects, and full visual/audio feedback.

**This is the THIRD major ability** integrating with our modular framework!

### System Components

```
Advanced Spitter System
│
├─► SpitterAbility (modular ability component)
│   └─► Handles targeting, cooldown, spawning
│
├─► ZombieProjectileBase (C++ base class)
│   ├─► Arc trajectory physics
│   ├─► Impact detection
│   ├─► Splash damage
│   ├─► Status effect application
│   └─► Puddle spawning
│
├─► Projectile Variants (configured via data table)
│   ├─► Acid Spit (direct damage + puddle)
│   ├─► Poison Spit (damage over time)
│   ├─► Slowing Spit (movement debuff)
│   ├─► Blinding Spit (vision impairment)
│   ├─► Explosive Spit (large splash)
│   └─► Fire Spit (burning effect)
│
└─► DamageOverTimePuddle (area denial)
    ├─► Damage-over-time zone
    ├─► Status effect application
    ├─► Visual feedback (decal + particles)
    └─► Audio feedback (sizzle sound)
```

---

## 📐 Architecture

### How It All Works Together:

```
1. Zombie with SpitterAbility detects target
   ↓
2. Ability activates when target in range
   ↓
3. Spawn projectile from mouth socket
   ↓
4. Projectile flies with arc trajectory
   ↓
5. On impact:
   ├─► Apply direct hit damage
   ├─► Apply splash damage in radius
   ├─► Apply status effect to hit actors
   └─► Spawn puddle at impact point
   ↓
6. Puddle persists and damages actors over time
   ↓
7. Puddle fades out after duration
```

---

## 🏗️ Core Components

### 1. SpitterAbility (Already Created!)

**File:** `Source/cleanzombie/Public/SpitterAbility.h`

**Purpose:** Modular ability component that handles spit attack logic.

**Key Features:**
- ✅ Target range checking (min/max)
- ✅ Cooldown management
- ✅ Projectile spawning
- ✅ Predictive targeting (leads moving targets)
- ✅ Integration with framework ability system
- ✅ Configurable per zombie variant

**Configuration:**
```cpp
SpitterAbility:
├─► ProjectileClass: BP_Projectile_Acid
├─► MinSpitRange: 300.0
├─► MaxSpitRange: 1500.0
├─► SpitCooldown: 3.0
├─► ProjectileDamage: 25.0
├─► ProjectileSpeed: 1000.0
└─► bPredictTargetMovement: true
```

### 2. ZombieProjectileBase

**File:** `Source/cleanzombie/Public/ZombieProjectileBase.h`

**Purpose:** Base C++ class for all spit projectiles with physics and effects.

**Components:**
```cpp
ZombieProjectileBase:
├─► CollisionSphere (15 unit radius)
├─► ProjectileMesh (visual)
├─► ProjectileMovement (arc physics)
├─► TrailEffect (particle trail)
└─► FlightAudio (whoosh sound)
```

**Key Features:**
- ✅ Arc trajectory calculation
- ✅ Configurable via data table
- ✅ Multiple projectile types
- ✅ Splash damage
- ✅ Status effect application
- ✅ Puddle spawning
- ✅ Multiplayer replicated
- ✅ Visual/audio feedback

**Data Table Configuration:**
```cpp
struct FProjectileConfigData:
├─► ProjectileType: Acid/Poison/Slowing/etc.
├─► DirectHitDamage: 25.0
├─► SplashDamage: 10.0
├─► SplashRadius: 150.0
├─► bCreatePuddle: true
├─► PuddleClass: BP_Puddle_Acid
├─► PuddleDuration: 10.0
├─► StatusEffectTag: "Poisoned"
├─► StatusEffectDuration: 5.0
├─► StatusEffectStrength: 1.0
├─► ProjectileMesh: SM_AcidBlob
├─► TrailEffect: PS_AcidTrail
├─► ImpactEffect: PS_AcidSplash
└─► ImpactSound: SFX_AcidImpact
```

### 3. DamageOverTimePuddle

**File:** `Source/cleanzombie/Public/DamageOverTimePuddle.h`

**Purpose:** Ground puddle that damages and applies effects over time.

**Components:**
```cpp
DamageOverTimePuddle:
├─► DamageTrigger (overlap box)
├─► PuddleDecal (ground texture)
├─► PuddleEffect (particles)
└─► SizzleAudio (ambient sound)
```

**Key Features:**
- ✅ Damage-over-time to overlapping actors
- ✅ Status effect application/refresh
- ✅ Configurable size, damage, duration
- ✅ Fade out animation
- ✅ Visual/audio feedback
- ✅ Tracks actors in puddle

**Configuration:**
```cpp
DamageOverTimePuddle:
├─► DamagePerTick: 5.0
├─► TickInterval: 0.5
├─► PuddleDuration: 10.0
├─► FadeOutDuration: 2.0
├─► PuddleRadius: 150.0
├─► StatusEffectTag: "Acidic"
├─► DecalMaterial: M_AcidPuddle
├─► ParticleTemplate: PS_PuddleBubbles
└─► SizzleSound: SFX_AcidSizzle
```

---

## 🚀 Quick Start (Blueprint Setup)

### Step 1: Create Projectile Data Table

1. **Create Data Table:**
   - Content Browser → Right-click → **Miscellaneous** → **Data Table**
   - Row Structure: **FProjectileConfigData**
   - Name: `DT_ProjectileTypes`

2. **Add Projectile Types:**

**Row: "AcidSpit"**
```
Projectile Type: Acid
Display Name: "Acid Spit"
Direct Hit Damage: 25.0
Splash Damage: 10.0
Splash Radius: 150.0
Create Puddle: ☑
Puddle Class: BP_Puddle_Acid
Puddle Duration: 10.0
Status Effect Tag: "Acidic"
Status Effect Duration: 0.0 (instant)
```

**Row: "PoisonSpit"**
```
Projectile Type: Poison
Display Name: "Poison Spit"
Direct Hit Damage: 15.0
Splash Damage: 5.0
Splash Radius: 100.0
Create Puddle: ☑
Puddle Class: BP_Puddle_Poison
Puddle Duration: 15.0
Status Effect Tag: "Poisoned"
Status Effect Duration: 8.0
```

**Row: "SlowingSpit"**
```
Projectile Type: Slowing
Display Name: "Slowing Spit"
Direct Hit Damage: 10.0
Splash Damage: 5.0
Splash Radius: 200.0
Create Puddle: ☑
Puddle Class: BP_Puddle_Slow
Puddle Duration: 8.0
Status Effect Tag: "Slowed"
Status Effect Duration: 5.0
```

**Row: "BlindingSpit"**
```
Projectile Type: Blinding
Display Name: "Blinding Spit"
Direct Hit Damage: 5.0
Splash Damage: 0.0
Splash Radius: 50.0
Create Puddle: ☐
Status Effect Tag: "Blinded"
Status Effect Duration: 3.0
```

### Step 2: Create Projectile Blueprint

1. **Create Blueprint Class:**
   - Parent: **ZombieProjectileBase**
   - Name: `BP_Projectile_Acid`

2. **Configure:**
   ```
   Projectile Config:
   ├─► Projectile Config Table: DT_ProjectileTypes
   ├─► Config Row Name: "AcidSpit"
   └─► (Auto-configures from data table)

   Visual Components:
   ├─► Projectile Mesh: (assign acid blob mesh)
   ├─► Trail Effect: (assign acid trail particle)
   └─► Material: M_AcidBlob
   ```

3. **Repeat** for other projectile types (Poison, Slowing, Blinding)

### Step 3: Create Puddle Blueprint

1. **Create Blueprint Class:**
   - Parent: **DamageOverTimePuddle**
   - Name: `BP_Puddle_Acid`

2. **Configure:**
   ```
   Puddle Damage:
   ├─► Damage Per Tick: 5.0
   ├─► Tick Interval: 0.5

   Puddle Lifetime:
   ├─► Puddle Duration: 10.0
   ├─► Fade Out Duration: 2.0

   Puddle Size:
   └─► Puddle Radius: 150.0

   Status Effect:
   ├─► Status Effect Tag: "Acidic"
   └─► Status Effect Strength: 1.0

   Visuals:
   ├─► Decal Material: M_AcidPuddle_Decal
   ├─► Particle Template: PS_PuddleBubbles
   └─► Sizzle Sound: SFX_AcidSizzle
   ```

3. **Create Decal Material:**
   - Base material with opacity
   - Green/acidic color
   - Animate with panner or noise
   - Expose "Opacity" parameter (for fade out)

4. **Create Particle Effect:**
   - Small bubbles rising from puddle
   - Subtle steam/smoke
   - Color matches projectile type

### Step 4: Add to Zombie

**In Data Table** (`DT_ZombieVariants`):

```
Row: "SpitterZombie"
├─► Variant Type: Custom
├─► Display Name: "Acid Spitter"
├─► Health: 80
├─► Speed: 250
├─► Abilities: [SpitterAbility, MultiTargetingAbility]
└─► Point Value: 60
```

**Or in Blueprint:**

```blueprint
ZombieBase:
├─► Add Component → Spitter Ability
│   └─► Configure:
│       ├─► Projectile Class: BP_Projectile_Acid
│       ├─► Min Spit Range: 300.0
│       ├─► Max Spit Range: 1500.0
│       └─► Spit Cooldown: 3.0
```

### Step 5: Set Up Mouth Socket

1. **Open Zombie Skeleton:**
   - Find zombie skeletal mesh
   - Open in Skeleton Editor

2. **Add Socket:**
   - Right-click on head bone
   - **Add Socket**
   - Name: `MouthSocket`
   - Position at mouth location
   - **Forward direction** should point where spit will go

3. **Test Socket Position:**
   - Preview with mesh attached
   - Adjust position/rotation

---

## 🎮 Integration with AI Behavior Toolkit

### Method 1: Animation Notify (Recommended)

1. **Open Spit Animation:**
   - Find/create zombie spit attack animation
   - Open in Animation Editor

2. **Add Anim Notify:**
   - Timeline → Right-click → **Add Notify** → **New Notify**
   - Name: `AnimNotify_SpawnProjectile`

3. **Implement Notify:**

```cpp
// AnimNotify_SpawnProjectile.h
UCLASS()
class UAnimNotify_SpawnProjectile : public UAnimNotify
{
    GENERATED_BODY()

public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override
    {
        if (AActor* Owner = MeshComp->GetOwner())
        {
            // Get Spitter Ability
            USpitterAbility* SpitterAbility = Owner->FindComponentByClass<USpitterAbility>();
            if (SpitterAbility)
            {
                SpitterAbility->SpitAtTarget();
            }
        }
    }
};
```

**Or in Blueprint:**
```blueprint
AnimNotify_SpawnProjectile (Blueprint):
├─► Get Owner
├─► Get Component By Class (SpitterAbility)
└─► Spit At Target
```

4. **Place Notify:**
   - Put at frame where spit leaves mouth
   - Usually 30-50% through animation

### Method 2: AttackRanged Behavior Integration

If using AI Behavior Toolkit's AttackRanged:

1. **In Behavior Component:**
   ```
   AttackRanged State:
   ├─► On Enter: Play Spit Animation
   ├─► On Animation Event: Spawn Projectile (via notify)
   └─► On Exit: Reset cooldown
   ```

2. **Add Cooldown Check:**
   ```blueprint
   Can Use AttackRanged?
   ├─► Get SpitterAbility
   ├─► Check: TimeSinceLastSpit >= SpitCooldown
   └─► Return: true/false
   ```

### Method 3: Direct Blueprint Logic

```blueprint
Zombie Event Tick:
├─► Has Target?
│   └─► YES:
│       ├─► Get SpitterAbility
│       ├─► Can Spit At Target?
│       │   └─► YES:
│       │       ├─► Is In Range?
│       │       │   └─► YES:
│       │       │       ├─► Play Spit Animation
│       │       │       └─► Animation Notify fires projectile
```

---

## 🎨 Projectile Variants Configuration

### Acid Spit (High Damage, Short Puddle)

```
Use Case: Direct damage dealer
├─► Direct Hit: 30 damage
├─► Splash: 15 damage, 150 radius
├─► Puddle: 10 seconds, 7 damage/tick
├─► Effect: Immediate damage
└─► Visual: Green glowing projectile, bubbling puddle
```

### Poison Spit (DoT, Long Puddle)

```
Use Case: Area denial, sustained damage
├─► Direct Hit: 15 damage
├─► Splash: 8 damage, 120 radius
├─► Puddle: 20 seconds, 5 damage/tick
├─► Effect: "Poisoned" tag, damage over 10s
└─► Visual: Purple cloud, toxic green puddle
```

### Slowing Spit (Movement Debuff)

```
Use Case: Crowd control, kiting prevention
├─► Direct Hit: 10 damage
├─► Splash: 5 damage, 250 radius
├─► Puddle: 12 seconds, 3 damage/tick
├─► Effect: "Slowed" tag, -50% movement for 6s
└─► Visual: Blue-white ice, frosty puddle
```

### Blinding Spit (Vision Impairment)

```
Use Case: Tactical advantage, panic inducement
├─► Direct Hit: 5 damage
├─► Splash: 2 damage, 100 radius
├─► Puddle: None
├─► Effect: "Blinded" tag, reduced vision for 4s
└─► Visual: Black ink cloud
```

### Explosive Spit (AOE)

```
Use Case: Multiple targets, structure damage
├─► Direct Hit: 20 damage
├─► Splash: 25 damage, 300 radius
├─► Puddle: 5 seconds, 10 damage/tick (fire)
├─► Effect: "Burning" tag
└─► Visual: Red-orange fireball, burning ground
```

---

## 🎯 Zombie Variant Examples

### Basic Spitter

```cpp
Row: "BasicSpitter"
├─► Health: 80
├─► Speed: 250
├─► Abilities: [SpitterAbility, MultiTargetingAbility]
└─► SpitterAbility Config:
    ├─► Projectile: BP_Projectile_Acid
    ├─► Range: 300-1500
    └─► Cooldown: 3.0s
```

**Behavior:** Keeps distance, spits acid at players.

### Sniper Spitter (Climber + Spitter)

```cpp
Row: "SniperSpitter"
├─► Health: 100
├─► Speed: 200
├─► Abilities: [ClimbingAbility, SpitterAbility, MultiTargetingAbility]
└─► Strategy:
    ├─► Climbs to high vantage points
    ├─► Spits poison from above
    └─► Drop attacks when reloading
```

**Behavior:** Climbs walls, attacks from elevated positions.

### Tank Spitter

```cpp
Row: "TankSpitter"
├─► Health: 250
├─► Speed: 150
├─► Abilities: [SpitterAbility]
└─► SpitterAbility Config:
    ├─► Projectile: BP_Projectile_Explosive
    ├─► Range: 400-1200
    └─► Cooldown: 5.0s
```

**Behavior:** Slow but powerful explosive spits.

### Support Spitter

```cpp
Row: "SupportSpitter"
├─► Health: 60
├─► Speed: 280
├─► Abilities: [SpitterAbility]
└─► SpitterAbility Config:
    ├─► Projectile: BP_Projectile_Slowing
    ├─► Range: 500-2000
    └─► Cooldown: 2.0s
```

**Behavior:** Slows players for other zombies to catch.

---

## 🎨 Visual & Audio Implementation

### Projectile Visuals

**Trail Particle System:**
```
PS_AcidTrail:
├─► Emitter: Trail renderer
├─► Color: Green → Dark Green
├─► Width: 10-30 units
├─► Lifetime: 0.5s
└─► Material: Additive, glowing
```

**Impact Particle System:**
```
PS_AcidSplash:
├─► Emitter 1: Burst splash
│   ├─► Count: 20-30 particles
│   ├─► Velocity: Outward, 100-300
│   └─► Color: Bright green
├─► Emitter 2: Smoke cloud
│   ├─► Count: 5-10 particles
│   └─► Lifetime: 2s
└─► Decal: Ground splatter (fades)
```

### Puddle Visuals

**Decal Material:**
```
M_AcidPuddle_Decal:
├─► Base Color: Toxic green
├─► Emissive: Slight glow
├─► Normal: Bubbling texture (panning)
├─► Opacity: Parameter "Opacity" (1.0 → 0.0)
└─► Blend Mode: Translucent
```

**Particle Effect:**
```
PS_PuddleBubbles:
├─► Spawn Rate: 5-10/second
├─► Velocity: Upward, 10-30
├─► Size: Small bubbles
├─► Color: Matches puddle type
└─► Lifetime: 1-2s
```

### Audio

**Projectile Launch:**
```
SFX_SpitLaunch:
├─► Sound: Wet "thwack" + whoosh
├─► Volume: 0.7
├─► Attenuation: 1500 units
└─► Pitch Variation: ±10%
```

**Projectile Flight:**
```
SFX_SpitFlight:
├─► Sound: Looping whoosh/sizzle
├─► Volume: 0.5
├─► Attenuation: 800 units
└─► Stops on impact
```

**Impact:**
```
SFX_SpitImpact:
├─► Sound: Wet splat + sizzle
├─► Volume: 0.8
├─► Attenuation: 1200 units
└─► Pitch based on surface type
```

**Puddle Ambient:**
```
SFX_PuddleSizzle:
├─► Sound: Looping acid sizzle
├─► Volume: 0.4
├─► Attenuation: 600 units
├─► Fades with puddle
└─► Stops on destruction
```

---

## 🔄 Multiplayer Replication

### What's Automatically Replicated:

✅ Projectile spawn (Actor replication)
✅ Projectile movement (ProjectileMovementComponent)
✅ Projectile type (Replicated property)
✅ Damage (Applied on server)
✅ Puddle spawn (Actor replication)
✅ Status effects (Tag system)

### Performance Optimization:

**For Many Spitters:**

1. **Projectile Pooling:**
   ```cpp
   Instead of: Destroy projectile on impact
   Use: Return to pool, reuse
   ```

2. **Reduce Update Frequency:**
   ```cpp
   ProjectileMovement->bForceSubStepping = false;
   ```

3. **LOD for Effects:**
   ```cpp
   If (DistanceToPlayer > 2000):
       Disable trail particles
       Reduce splash effect complexity
   ```

4. **Puddle Culling:**
   ```cpp
   If (DistanceToPlayer > 3000):
       Don't spawn particle effect
       Reduce tick frequency
   ```

---

## 📊 Status Effect System Integration

### Simple Tag-Based (Current Implementation)

```cpp
// Apply status effect
Actor->Tags.AddUnique("Poisoned");

// Check status effect
if (Actor->Tags.Contains("Poisoned"))
{
    // Apply poison damage/effect
}

// Remove after duration
FTimerHandle Timer;
GetWorld()->GetTimerManager().SetTimer(Timer, [Actor]() {
    Actor->Tags.Remove("Poisoned");
}, Duration, false);
```

### Advanced Status Effect Component (Included!)

**Full implementation included in:** `Source/cleanzombie/Public/StatusEffectComponent.h`

The status effect system provides:
- **Multiple Effect Types**: Poison, Acid, Fire, Slowing, Blinding, Stun, Weakness
- **Damage over Time**: Automatic DoT application with configurable tick rates
- **Effect Stacking**: Stack effects up to maximum limits
- **Movement Modifiers**: Automatic speed reduction for slow/stun effects
- **Damage Modifiers**: Weakness effect reduces damage output
- **Duration Management**: Automatic expiration and cleanup
- **Blueprint Integration**: Full Blueprint support with events

**Effect Types:**
```cpp
enum class EStatusEffectType : uint8
{
    None,
    Poison,      // Damage over time
    Acid,        // Armor/health debuff + DoT
    Slowing,     // Movement speed reduction
    Blinding,    // Vision impairment
    Fire,        // High DoT
    Stun,        // Complete immobilization
    Weakness,    // Damage output reduction
    Custom       // For custom Blueprint effects
};
```

**Usage in C++:**
```cpp
// Apply poison effect
UStatusEffectComponent* StatusComp = Target->FindComponentByClass<UStatusEffectComponent>();
if (StatusComp)
{
    StatusComp->ApplyStatusEffect(EStatusEffectType::Poison, 1.0f, 8.0f, ProjectileOwner);
}

// Check for effects
if (StatusComp->HasStatusEffect(EStatusEffectType::Slowing))
{
    float SpeedMult = StatusComp->GetMovementSpeedMultiplier();
    // Apply to character movement
}

// Get all active effects
TArray<FStatusEffect> Effects = StatusComp->GetAllActiveEffects();
```

**Usage in Blueprints:**
1. Add `StatusEffectComponent` to your Character/Pawn Blueprint
2. Call `Apply Status Effect` when hit by projectile or standing in puddle
3. Query effects: `Has Status Effect`, `Is Stunned`, `Is Blinded`
4. Get multipliers: `Get Movement Speed Multiplier`, `Get Damage Output Multiplier`
5. Override events: `On Status Effect Applied`, `On Status Effect Removed`, `On DoT Damage Dealt`

**Adding to Player/NPC:**
```cpp
// In your Character class
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status Effects")
UStatusEffectComponent* StatusEffects;

// Constructor
StatusEffects = CreateDefaultSubobject<UStatusEffectComponent>(TEXT("StatusEffects"));
```

**Effect Configuration:**
The component uses configurable defaults:
- `DefaultPoisonDamage` = 5.0 damage per tick
- `DefaultAcidDamage` = 8.0 damage per tick
- `DefaultFireDamage` = 10.0 damage per tick
- `DefaultSlowPercentage` = 0.5 (50% speed reduction)
- `DefaultBlindStrength` = 0.7 (70% vision reduction)

**Automatic Integration:**
The projectile and puddle systems automatically detect and use StatusEffectComponent:
```cpp
// In ZombieProjectileBase::ApplyStatusEffect()
// Automatically uses StatusEffectComponent if available, falls back to tags

// In DamageOverTimePuddle::ApplyDamageToActorsInPuddle()
// Automatically applies status effects to actors with StatusEffectComponent
```

### Status Effect Implementations

**Poison:**
```cpp
Effect: "Poisoned"
├─► Damage: 3/second for 8 seconds
├─► Visual: Green overlay, damage numbers
└─► Sound: Coughing, sizzle
```

**Slowing:**
```cpp
Effect: "Slowed"
├─► Movement: -50% speed for 5 seconds
├─► Visual: Blue tint, ice particles
└─► Sound: Crunchy footsteps
```

**Blinding:**
```cpp
Effect: "Blinded"
├─► Vision: Reduced visibility (post-process)
├─► Duration: 3 seconds
├─► Visual: Dark vignette, blur
└─► Sound: Ringing
```

---

## 🐛 Troubleshooting

### "Projectile Doesn't Spawn"

**Fixes:**
1. Check mouth socket exists: `MouthSocket`
2. Verify projectile class assigned
3. Check cooldown not active
4. Enable `bShowDebug` on SpitterAbility
5. Check animation notify fires

### "Projectile Goes Straight (No Arc)"

**Fixes:**
1. Set `ProjectileGravityScale > 0` (default 0.5)
2. Use `FireWithArc()` instead of `FireInDirection()`
3. Increase `ArcHeight` parameter

### "Puddle Doesn't Appear"

**Fixes:**
1. Check `bCreatePuddle = true`
2. Verify `PuddleClass` assigned
3. Check puddle spawns at impact point
4. Enable `bShowDebug` on projectile

### "Status Effect Doesn't Apply"

**Fixes:**
1. Check `StatusEffectTag` not empty
2. Verify target has tag after hit
3. Implement status effect logic in player/NPC
4. Use `UStatusEffectComponent` for advanced effects

### "Performance Issues with Many Spitters"

**Fixes:**
1. Increase `SpitCooldown` (reduce fire rate)
2. Reduce `SplashRadius` (fewer overlap checks)
3. Disable trail particles for distant projectiles
4. Pool projectiles instead of spawning new ones
5. Reduce puddle `TickInterval` (0.5s → 1.0s)

---

## 🎓 Best Practices

### Do's ✅

- **Use data tables** for projectile configuration
- **Configure per variant** for variety
- **Test arc trajectory** in editor
- **Provide visual feedback** (players must see incoming spit)
- **Balance cooldowns** (not too spammy)
- **Create variety** (acid, poison, slow, blind)

### Don'ts ❌

- **Don't make puddles permanent** (performance!)
- **Don't forget line of sight** (no shooting through walls)
- **Don't spam too fast** (not fun to play against)
- **Don't make effects invisible** (frustrating)
- **Don't forget multiplayer testing**

---

## 📋 Quick Reference

### Adding New Projectile Type:

1. Add row to `DT_ProjectileTypes`
2. Create Blueprint from `ZombieProjectileBase`
3. Assign config row
4. Create puddle Blueprint if needed
5. Assign to zombie variant

### Changing Spit Behavior:

- **More damage:** Increase `DirectHitDamage`
- **Larger splash:** Increase `SplashRadius`
- **Longer puddles:** Increase `PuddleDuration`
- **Faster fire rate:** Reduce `SpitCooldown`
- **Longer range:** Increase `MaxSpitRange`

---

## 🚀 Integration Status

**What We Have Now:**

| System | Status | Integration |
|--------|--------|-------------|
| Framework | ✅ Complete | Foundation |
| Climbing | ✅ Complete | Ability 1 |
| Multi-Targeting | ✅ Complete | Ability 2 |
| **Advanced Spitter** | ✅ Complete | **Ability 3** |

**Ability Combinations:**
```cpp
Sniper Zombie:
└─► [ClimbingAbility + SpitterAbility + MultiTargetingAbility]
    = Climbs walls, spits from above, smart targeting!

Support Zombie:
└─► [SpitterAbility + MultiTargetingAbility]
    = Slows players, prioritizes runners!

Elite Zombie:
└─► [ClimbingAbility + SpitterAbility + MultiTargetingAbility]
    = Everything! Ultimate threat!
```

---

## 📚 Related Documentation

- [ZOMBIE_FRAMEWORK_GUIDE.md](ZOMBIE_FRAMEWORK_GUIDE.md) - Framework overview
- [CLIMBING_SYSTEM_GUIDE.md](CLIMBING_SYSTEM_GUIDE.md) - Climbing ability
- [MULTI_TARGETING_GUIDE.md](MULTI_TARGETING_GUIDE.md) - Multi-target system

---

**You now have an advanced ranged attack system with 6 projectile types, puddles, status effects, and full integration with the modular framework!** 🧟‍♂️💥

**Next Steps:**
1. Compile C++ code
2. Create data tables
3. Create projectile Blueprints
4. Create puddle Blueprints
5. Add to zombie variants
6. Test and balance!
