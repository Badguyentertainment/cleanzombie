# 🧟 WWG Mutating Zombie Default - Integration Guide

**Merging Professional Asset with Custom Ability System**

---

## 📋 Overview

This guide integrates the **WWG Mutating Zombie Default** marketplace asset with our custom modular zombie ability system, creating a powerful hybrid that combines:

✅ **WWG Features:** Progressive transformation, dismemberment, ARKit facials, dirt system
✅ **Our Systems:** Modular abilities, climbing, tunneling, spitting, targeting, status effects

---

## ⚡ Quick Setup (5 Minutes)

### Step 1: Import WWG Asset

1. Add **WWG_ZombieDefault** from Marketplace to project
2. Let Unreal import all assets
3. Locate in Content Browser: `/Game/WWG_ZombieDefault/`

### Step 2: Choose Integration Method

**Option A: Component Method** (Recommended - No Blueprint Changes)
```
Keep WWG zombie as-is
Add UWWGZombieIntegration component
Add ability components
Done!
```

**Option B: Reparent Method** (More integrated)
```
Reparent WWG Blueprint to AWWGZombieBase
Abilities built-in
Tighter integration
```

### Step 3: Add Our Abilities

```
Open WWG Zombie Blueprint
Add Components:
  ├─ WWGZombieIntegration (if using Option A)
  ├─ ClimbingAbility
  ├─ TunnelNavigationAbility
  ├─ SpitterAbility
  ├─ MultiTargetingAbility
  └─ StatusEffectComponent
```

---

## 🏗️ Integration Architecture

### System Merge Strategy

```
WWG Mutating Zombie Default
    ├─ Skeletal Mesh (Epic Skeleton) ✓ Keep
    ├─ Transformation System ✓ Keep + Enhance
    ├─ Dismemberment System ✓ Keep + Link to Abilities
    ├─ ARKit Blendshapes ✓ Keep
    ├─ Dirt/Decay Shader ✓ Keep
    └─ Modular Parts ✓ Keep

Our Custom Systems (ADD)
    ├─ ZombieAbilityComponent Framework
    ├─ Climbing/Ceiling Navigation
    ├─ Tunnel Traversal
    ├─ Spitter with Projectiles
    ├─ Multi-Targeting (NPCs, Barricades)
    └─ Status Effects (Poison, Fire, etc.)

Integration Layer (NEW)
    └─ UWWGZombieIntegration
         ├─ Bridges zombification with abilities
         ├─ Links dismemberment to ability blocking
         └─ Synchronizes systems
```

---

## 🎯 Method A: Component Integration (Recommended)

**Best for:** Keeping WWG asset unchanged, easy updates

### Setup

**1. Open WWG Zombie Blueprint**
```
Content/WWG_ZombieDefault/Blueprints/BP_Zombie_Default
(or whatever the WWG zombie is called)
```

**2. Add Components**

In Components panel:
```
Add Component → WWGZombieIntegration
Add Component → ClimbingAbility
Add Component → TunnelNavigationAbility
Add Component → SpitterAbility
Add Component → MultiTargetingAbility
Add Component → StatusEffectComponent
```

**3. Configure WWG Integration**

Select WWGZombieIntegration in components:
```
Zombification:
  ├─ Zombification Level: 1.0 (full zombie by default)
  ├─ Zombification Rate: 0.1 (if using infection mechanic)
  ├─ Is Infected: false
  └─ Can Cure Infection: false (zombies can't be cured)

Dismemberment:
  ├─ Dismember Chance: 0.3 (30% on heavy damage)
  ├─ Min Dismember Damage: 50.0
  ├─ Can Survive Dismemberment: true

Appearance:
  ├─ Dirt Level: 0.7 (fairly dirty)
  ├─ Accumulate Dirt: true
  └─ Dirt Accumulation Rate: 0.01
```

**4. Link WWG Events to Abilities**

Event Graph:
```cpp
// When zombification changes, update abilities
Event On Zombification Changed:
  ├─ Get WWG Integration
  └─ Apply Zombification To Abilities

// When limb dismembered, disable relevant abilities
Event On Limb Dismembered:
  ├─ Branch (Limb Name == "LeftArm" OR "RightArm")
  │    True:
  │      ├─ Get Climbing Ability
  │      └─ Deactivate Ability  // Can't climb without arms
  ├─ Branch (Limb Name == "LeftLeg" OR "RightLeg")
  │    True:
  │      ├─ Get Tunnel Navigation Ability
  │      └─ Deactivate Ability  // Hard to crawl without legs
  └─ Apply Dismemberment To Abilities
```

---

## 🔄 Method B: Reparent Integration

**Best for:** Maximum integration, custom zombie variants

### Setup

**1. Reparent WWG Blueprint**

```
Right-click WWG Zombie Blueprint
Blueprint Props → Parent Class
Search: WWGZombieBase
Reparent
```

**2. Configure in Constructor**

Now the zombie has built-in WWGIntegration component!

**3. Add Abilities in Blueprint**

```
Components panel:
  WWGIntegration (inherited ✓)
  Add → ClimbingAbility
  Add → TunnelNavigationAbility
  Add → SpitterAbility
  Add → MultiTargetingAbility
  Add → StatusEffectComponent
```

**4. Override Virtual Functions**

```cpp
// Customize zombification behavior
Event On Fully Zombified:
  ├─ Enable All Combat Abilities
  ├─ Set Max Movement Speed
  └─ Play Transformation Complete Anim

// Customize dismemberment behavior
Event On Limb Dismembered:
  ├─ Spawn Gore Particles
  ├─ Play Dismember Sound
  ├─ Check If Should Die
  └─ Update Ability States
```

---

## 🧬 Zombification + Abilities System

### Progressive Ability Unlocking

**Concept:** Abilities get stronger as zombification increases

**Implementation:**

```cpp
// In Ability Blueprint

Function: Get Ability Effectiveness
  ├─ Get WWG Integration
  ├─ Get Zombification Level
  ├─ Map to Effectiveness:
  │    0.0 (Human)  → 50% effectiveness
  │    0.5 (Half)   → 75% effectiveness
  │    1.0 (Full)   → 100% effectiveness
  └─ Return Multiplier

Event Update Ability:
  ├─ Base Damage: 25
  ├─ Get Ability Effectiveness
  └─ Final Damage = Base * Effectiveness
```

### Example: Climbing Improves with Mutation

```cpp
ClimbingAbility Blueprint:

Event Activate Ability:
  ├─ Get WWG Integration
  ├─ Get Zombification Level
  ├─ Speed Bonus = Zombification * 50
  │    └─ 0% zombie = +0 speed
  │        50% zombie = +25 speed
  │        100% zombie = +50 speed
  ├─ Set Climb Speed (150 + Speed Bonus)
  └─ More zombie = Better climber!
```

### Example: Dirt Affects Stealth

```cpp
MultiTargetingAbility Blueprint:

Function: Calculate Detection Range
  ├─ Get WWG Integration
  ├─ Get Dirt Level
  ├─ Base Range: 2000
  ├─ Dirt Penalty = Dirt Level * 500
  │    └─ Clean (0.0) = +0 range penalty
  │        Dirty (0.5) = +250 range
  │        Filthy (1.0) = +500 range
  └─ Detection Range = Base + Dirt Penalty
       (Dirty zombies are easier to spot!)
```

---

## 💀 Dismemberment + Ability Blocking

### Automatic Ability Disabling

The integration automatically blocks abilities when limbs are lost:

| Dismembered Limb | Disabled Abilities |
|------------------|-------------------|
| Left/Right Arm | Climbing, Tunnel Crawling |
| Left/Right Leg | Tunnel Navigation, Sprinting |
| Head | ALL (instant death) |
| Spine | ALL (instant death) |

### Custom Dismemberment Rules

```cpp
Event On Limb Dismembered:
  ├─ Get Limb Name
  ├─ Switch on Limb Name:
  │    "LeftArm":
  │      ├─ Get Climbing Ability
  │      ├─ Reduce Climb Speed 50%
  │      └─ Can still climb with one arm
  │
  │    "RightArm":
  │      ├─ If Left Arm Also Missing:
  │      │    └─ Disable Climbing Completely
  │      └─ Else: Reduce speed 50%
  │
  │    "LeftLeg":
  │      ├─ Reduce Movement Speed 30%
  │      ├─ Get Tunnel Navigation
  │      └─ Slow down tunnel traversal
  │
  │    "RightLeg":
  │      ├─ If Left Leg Also Missing:
  │      │    ├─ Crawl Only (very slow)
  │      │    └─ Disable Tunnel Navigation
  │      └─ Else: Reduce speed 30%
  └─ Update Animation State
```

### One-Armed Climber Example

```cpp
ClimbingAbility:

Event Try Start Climbing:
  ├─ Get WWG Integration
  ├─ Check Arms:
  │    Both Arms OK: Climb Speed 200
  │    One Arm Missing: Climb Speed 100
  │    Both Arms Missing: Cannot Climb
  └─ Set Climbing Parameters
```

---

## 🎨 Visual Integration

### Transformation Visual Feedback

**WWG provides zombification float (0-1), use it for:**

1. **Mesh Morphing** (WWG handles automatically)
2. **Material Changes** (WWG handles automatically)
3. **Particle Effects** (We add)

```cpp
Event On Zombification Changed:
  ├─ Branch (New Level > 0.75):
  │    True: Spawn Decay Particles
  ├─ Branch (New Level > 0.5):
  │    True: Enable Pulsing Veins
  ├─ Branch (New Level > 0.25):
  │    True: Bloodshot Eyes
  └─ Update Material Parameters
```

### Dismemberment Visual Enhancement

**WWG provides gore system, enhance with our effects:**

```cpp
Event On Limb Dismembered:
  ├─ WWG System: Detach Mesh, Show Stump
  ├─ Our Addition:
  │    ├─ Spawn Blood Fountain (Niagara)
  │    ├─ Blood Pool Decal
  │    ├─ Limb Ragdoll Physics
  │    ├─ Camera Shake
  │    └─ Slow Motion (0.3s)
  └─ Play Gore Sound
```

---

## 🎮 Gameplay Integration Examples

### Example 1: Infection Spread Mechanic

**Zombies infect players on hit, players slowly transform**

```cpp
Zombie Attack Hits Player:
  ├─ Damage Player
  ├─ Get Player's WWGIntegration (if they have one)
  ├─ Branch (Player has integration):
  │    True:
  │      ├─ Infect Character (0.1 initial level)
  │      ├─ Show UI: "YOU'VE BEEN INFECTED"
  │      └─ Start Transformation Timer
  └─ Player must find cure before full transformation!

Player Transformation Tick:
  ├─ Get Zombification Level
  ├─ Branch (Level > 0.5):
  │    True:
  │      ├─ Vision Distortion
  │      ├─ Movement Changes
  │      └─ UI: "RESIST THE INFECTION"
  ├─ Branch (Level >= 1.0):
  │    True:
  │      ├─ Player Becomes AI-Controlled Zombie
  │      ├─ Transfer to Enemy Team
  │      └─ Achievement: "One of Them"
  └─ Update Health Bar Color (human → zombie)
```

### Example 2: Progressive Zombie Variants

**Different zombie types at different infection stages**

```cpp
Spawn Zombie Variant:
  ├─ Spawn WWG Zombie
  ├─ Get WWG Integration
  ├─ Roll Random Variant:
  │
  │    Variant: "Shambler" (Early stage)
  │      ├─ Set Zombification: 0.3
  │      ├─ Add Abilities: None
  │      ├─ Slow Movement
  │      └─ Low Damage
  │
  │    Variant: "Runner" (Mid stage)
  │      ├─ Set Zombification: 0.6
  │      ├─ Add Abilities: MultiTargeting
  │      ├─ Fast Movement
  │      └─ Medium Damage
  │
  │    Variant: "Spitter" (Late stage)
  │      ├─ Set Zombification: 0.8
  │      ├─ Add Abilities: SpitterAbility
  │      ├─ Medium Movement
  │      └─ Ranged Attack
  │
  │    Variant: "Crawler" (Dismembered)
  │      ├─ Set Zombification: 1.0
  │      ├─ Dismember Both Legs
  │      ├─ Crawl Only
  │      ├─ Add Abilities: TunnelNavigation
  │      └─ High Damage (desperate)
  │
  │    Variant: "Climber" (Full mutation)
  │      ├─ Set Zombification: 1.0
  │      ├─ Add Abilities: ClimbingAbility
  │      ├─ Medium Movement
  │      └─ Ambush from Above
  └─ Spawn into Level
```

### Example 3: Dismemberment Tactics

**Target specific limbs for tactical advantages**

```cpp
Weapon: Shotgun

Event Hit Zombie:
  ├─ Get Hit Bone Name
  ├─ Calculate Damage (100 base)
  ├─ Get WWG Integration
  ├─ Branch (Hit Bone is Limb):
  │    True:
  │      ├─ Roll Dismember Check
  │      ├─ Dismember Limb
  │      └─ Achievement: "Disarmed"
  ├─ Branch (Hit Bone == "Head"):
  │    True:
  │      ├─ Critical Hit (3x damage)
  │      ├─ Dismember Head
  │      └─ Instant Kill
  └─ Apply Damage

Player Strategy:
  - Shoot legs → Slow zombie down
  - Shoot arms → Disable climbing
  - Shoot head → Instant kill
```

### Example 4: Decay System

**Zombies rot over time, affecting performance**

```cpp
Zombie Lifespan System:

Variable: Time Since Spawn

Event Tick:
  ├─ Increment Time Since Spawn
  ├─ Get WWG Integration
  ├─ Calculate Decay:
  │    ├─ Decay Level = Time / 600 seconds (10 min)
  │    ├─ Set Dirt Level (Decay Level)
  │    └─ Visual: More rotten over time
  ├─ Apply Decay Effects:
  │    ├─ Movement Speed *= (1.0 - Decay * 0.3)
  │    ├─ Health -= Decay * 5/sec
  │    └─ Limb HP Reduced
  ├─ Branch (Decay > 0.8):
  │    True:
  │      ├─ Random Limb Falls Off
  │      ├─ Spawn Maggots Particle
  │      └─ Play Squelch Sound
  └─ Branch (Decay >= 1.0):
       True: Zombie Collapses (death)
```

---

## 🔧 Status Effects + Dismemberment

### Fire Damage Accelerates Decay

```cpp
StatusEffectComponent:

Event On Status Effect Applied (Fire):
  ├─ Get WWG Integration
  ├─ Increase Dirt Level (+0.5)
  │    └─ Burnt = Very dirty visually
  ├─ Increase Dismember Chance (+0.2)
  │    └─ Burnt flesh easier to tear
  └─ Spawn Burning Particles
```

### Acid Melts Limbs

```cpp
StatusEffectComponent:

Event On DoT Damage Dealt (Acid):
  ├─ Accumulate Acid Damage
  ├─ Branch (Acid Damage > 100):
  │    True:
  │      ├─ Get WWG Integration
  │      ├─ Select Random Limb
  │      ├─ Dismember Limb (acid dissolved it)
  │      ├─ Spawn Acid Dissolve VFX
  │      └─ Reset Acid Damage
  └─ Corrode Visual Effect
```

### Ice Shatters Limbs

```cpp
Custom Status Effect: Frozen

While Frozen:
  ├─ Movement Speed = 0
  ├─ Ice Material Override
  └─ Vulnerable to Shatter

On Hit While Frozen:
  ├─ Branch (Damage > 30):
  │    True:
  │      ├─ Get WWG Integration
  │      ├─ Dismember All Limbs
  │      ├─ Spawn Ice Shards
  │      ├─ Camera Shake
  │      └─ Instant Kill
  └─ Shatter Achievement
```

---

## 📊 Data Table Integration

### Zombie Variant Configuration

Create DataTable: **DT_ZombieVariants**

```cpp
Row: Shambler
  ├─ Zombie Variant: Basic
  ├─ Zombification Level: 0.3
  ├─ Dirt Level: 0.4
  ├─ Max Health: 100
  ├─ Abilities:
  │    └─ (None)
  └─ Dismembered Limbs: []

Row: Runner
  ├─ Zombie Variant: Fast
  ├─ Zombification Level: 0.6
  ├─ Dirt Level: 0.5
  ├─ Max Health: 80
  ├─ Abilities:
  │    └─ MultiTargetingAbility
  └─ Dismembered Limbs: []

Row: Climber
  ├─ Zombie Variant: Wall
  ├─ Zombification Level: 0.85
  ├─ Dirt Level: 0.7
  ├─ Max Health: 120
  ├─ Abilities:
  │    └─ ClimbingAbility
  └─ Dismembered Limbs: []

Row: Spitter
  ├─ Zombie Variant: Ranged
  ├─ Zombification Level: 0.9
  ├─ Dirt Level: 0.8
  ├─ Max Health: 90
  ├─ Abilities:
  │    ├─ SpitterAbility
  │    └─ MultiTargetingAbility
  └─ Dismembered Limbs: []

Row: Crawler
  ├─ Zombie Variant: Dismembered
  ├─ Zombification Level: 1.0
  ├─ Dirt Level: 1.0
  ├─ Max Health: 60
  ├─ Abilities:
  │    └─ TunnelNavigationAbility
  └─ Dismembered Limbs: [LeftLeg, RightLeg]

Row: Tunneler
  ├─ Zombie Variant: Vent
  ├─ Zombification Level: 0.75
  ├─ Dirt Level: 0.9
  ├─ Max Health: 110
  ├─ Abilities:
  │    └─ TunnelNavigationAbility
  └─ Dismembered Limbs: []
```

### Spawn from Data Table

```cpp
Function: Spawn Zombie From Config

Input: Row Name (e.g., "Spitter")

  ├─ Get DataTable Row (DT_ZombieVariants)
  ├─ Spawn WWG Zombie
  ├─ Get WWG Integration
  ├─ Set Zombification Level (from row)
  ├─ Set Dirt Level (from row)
  ├─ Set Max Health (from row)
  ├─ For Each Ability Class:
  │    └─ Add Ability Component
  ├─ For Each Dismembered Limb:
  │    └─ Dismember Limb
  └─ Return Spawned Zombie
```

---

## 🎬 Cinematic Integration

### Transformation Sequence

```cpp
Cinematic: Player Infection Scene

Timeline:
  0:00 - Player Bitten
  0:02 - Close-up on bite
  0:03 - Infection starts (Zombification 0.1)
  0:05 - Veins darken
  0:08 - Eyes turn white (Zombification 0.3)
  0:12 - Skin pales (Zombification 0.5)
  0:15 - Teeth sharpen (Zombification 0.7)
  0:18 - Full transformation (Zombification 1.0)
  0:20 - Zombie roar, cut to gameplay

Implementation:
  ├─ Matinee/Sequencer Timeline
  ├─ Keyframe Zombification Level
  ├─ WWG asset morphs automatically
  ├─ Add particle effects at keyframes
  └─ Trigger gameplay at end
```

### Dismemberment Kill Cam

```cpp
On Critical Dismember:
  ├─ Freeze Time (0.1x)
  ├─ Camera: Follow Limb
  ├─ Highlight: Limb Separation Point
  ├─ Particle: Blood Spray (Niagara)
  ├─ Sound: Bone Crack + Gore
  ├─ Resume Time (1.0x after 2s)
  └─ Achievement Popup (if headshot)
```

---

## ⚙️ Performance Optimization

### LOD with Dismemberment

```cpp
Distance LOD System:

< 500 units (Close):
  ├─ Full Detail Mesh
  ├─ All Dismemberment Active
  ├─ Facial Animations (ARKit)
  └─ Dirt Shader (Full Quality)

500-1500 units (Medium):
  ├─ LOD 1 Mesh
  ├─ Major Dismemberment Only
  ├─ No Facial Animations
  └─ Dirt Shader (Medium Quality)

> 1500 units (Far):
  ├─ LOD 2 Mesh
  ├─ No Dismemberment
  ├─ Static Face
  └─ Simple Diffuse Material
```

### Memory Management

```cpp
Dismembered Limb Cleanup:

On Limb Dismembered:
  ├─ Spawn Detached Limb Actor
  ├─ Set Lifespan (30 seconds)
  ├─ After 30s: Destroy Limb
  └─ Prevents memory leak from 1000+ limbs
```

---

## 📝 Summary

### Integration Checklist

✅ **Step 1:** Import WWG_ZombieDefault asset
✅ **Step 2:** Choose integration method (Component or Reparent)
✅ **Step 3:** Add WWGZombieIntegration component
✅ **Step 4:** Add ability components (Climbing, Tunneling, Spitter, etc.)
✅ **Step 5:** Link zombification to ability effectiveness
✅ **Step 6:** Link dismemberment to ability blocking
✅ **Step 7:** Configure variant data tables
✅ **Step 8:** Test transformation system
✅ **Step 9:** Test dismemberment system
✅ **Step 10:** Test all abilities with WWG zombie

### What You Get

🧟 **Professional Zombie Asset:** WWG's high-quality transforming zombie
🎮 **Modular Abilities:** Our custom climbing, tunneling, spitting systems
💀 **Dismemberment:** Tactical limb destruction affecting abilities
🧬 **Transformation:** Progressive zombification with gameplay impact
🎨 **Visual Polish:** Dirt, decay, gore all integrated
🔥 **Status Effects:** Fire, acid, poison work with dismemberment
📊 **Data-Driven:** Configure variants without code
🌐 **Multiplayer:** Everything replicated and synchronized

### Best Practices

1. **Use Component Method** for easy WWG asset updates
2. **Configure variants** via data tables
3. **Link zombification** to ability power scaling
4. **Use dismemberment** for tactical gameplay
5. **Optimize LOD** for performance with many zombies
6. **Clean up gore** to prevent memory issues
7. **Test multiplayer** early and often

---

**You now have a AAA-quality zombie with advanced gameplay systems!** 🎮🧟💪

