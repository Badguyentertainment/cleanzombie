# Animation Blueprint Setup Guide for Climbing Zombies

## Overview

This guide provides detailed instructions for setting up animations for the zombie climbing system. It covers Animation Blueprint configuration, state machines, and how to create or adapt animations for wall and ceiling climbing.

---

## Table of Contents

1. [Animation Requirements](#animation-requirements)
2. [Animation Blueprint Setup](#animation-blueprint-setup)
3. [State Machine Configuration](#state-machine-configuration)
4. [Animation Blending](#animation-blending)
5. [Retargeting Existing Animations](#retargeting-existing-animations)
6. [Creating New Climbing Animations](#creating-new-climbing-animations)
7. [Testing and Debugging](#testing-and-debugging)

---

## Animation Requirements

### Minimum Required Animations

Your zombie needs these animation types:

| Animation Type | Usage | Can Reuse? |
|---------------|-------|-----------|
| **Wall Climb Idle** | Stationary on wall | ✓ Use first frame of walk cycle |
| **Wall Climb Move** | Climbing up/along walls | ✓ Adapt ZombieWalking |
| **Ceiling Idle** | Hanging from ceiling | ✓ Use first frame of crawl |
| **Ceiling Move** | Crawling on ceiling | ✓ Use ZombieCrawl rotated |
| **Climb To Wall** | Transition from ground | ⚠ New or blend existing |
| **Drop From Surface** | Fall transition | ⚠ New or use falling anim |

### Optional Enhanced Animations

| Animation Type | Usage | Priority |
|---------------|-------|----------|
| **Wall to Ceiling Transition** | Moving from wall to ceiling | Medium |
| **Ceiling to Wall Transition** | Moving from ceiling to wall | Medium |
| **Wall Attack** | Attack while on wall | Low |
| **Ceiling Attack** | Drop attack from ceiling | Medium |
| **Wall Strafe Left/Right** | Side movement on walls | Low |

---

## Animation Blueprint Setup

### Step 1: Create Zombie Climbing Animation Blueprint

If you don't have a separate Animation BP for climbing zombies:

1. **Duplicate existing zombie Animation BP:**
   - Right-click on `Content/HGT/AI_Zombie/ZombieAnimations/AnimBP_Zombie` (if exists)
   - Select **Duplicate**
   - Rename to `AnimBP_ZombieClimbing`

   OR

2. **Create new Animation Blueprint:**
   - Right-click in Content Browser → **Animation** → **Animation Blueprint**
   - Parent Class: `AnimInstance`
   - Target Skeleton: Your zombie skeleton
   - Name: `AnimBP_ZombieClimbing`

### Step 2: Create Animation Blueprint Variables

Open your Animation Blueprint and create these variables:

#### Core Variables

```
Variable Name: bIsClimbing
├── Type: Boolean
├── Category: Climbing
├── Default: false
└── Tooltip: "Is the zombie currently climbing?"

Variable Name: ClimbingSurfaceType
├── Type: Byte (or Enum if EClimbingSurfaceType is exposed)
├── Category: Climbing
├── Default: 0 (None)
└── Tooltip: "Current surface type: 0=None, 1=Wall, 2=Ceiling, 3=Floor"

Variable Name: ClimbSpeed
├── Type: Float
├── Category: Climbing
├── Default: 0.0
└── Tooltip: "Current climbing speed magnitude"

Variable Name: ClimbDirection
├── Type: Vector
├── Category: Climbing
├── Default: (0, 0, 0)
└── Tooltip: "Direction zombie is climbing in"

Variable Name: bIsOnCeiling
├── Type: Boolean
├── Category: Climbing
├── Default: false
└── Tooltip: "Helper variable - true when on ceiling"

Variable Name: bIsOnWall
├── Type: Boolean
├── Category: Climbing
├── Default: false
└── Tooltip: "Helper variable - true when on wall"
```

#### Optional Variables

```
Variable Name: TimeInClimbingState
├── Type: Float
├── Category: Climbing|Debug
└── Tooltip: "How long in current climbing state (for blending)"

Variable Name: PreviousMovementMode
├── Type: Byte
├── Category: Climbing|Debug
└── Tooltip: "Previous movement mode before climbing"
```

### Step 3: Update Animation Variables (Event Graph)

In the **Event Graph** of your Animation BP:

```blueprint
Event Blueprint Update Animation
│
├─► Try Get Pawn Owner
│   │
│   ├─► Cast to Character (Success)
│   │   │
│   │   ├─► Get Component By Class
│   │   │   Class: ZombieClimbingMovementComponent
│   │   │   │
│   │   │   ├─► Is Valid? (Check if component exists)
│   │   │   │   │
│   │   │   │   ├─► [TRUE PATH]
│   │   │   │   │   │
│   │   │   │   │   ├─► Is Climbing
│   │   │   │   │   │   └─► SET bIsClimbing
│   │   │   │   │   │
│   │   │   │   │   ├─► Get Current Surface Type
│   │   │   │   │   │   └─► SET ClimbingSurfaceType
│   │   │   │   │   │
│   │   │   │   │   ├─► Get Velocity
│   │   │   │   │   │   └─► Vector Length
│   │   │   │   │   │       └─► SET ClimbSpeed
│   │   │   │   │   │
│   │   │   │   │   ├─► Get Current Surface Normal
│   │   │   │   │   │   └─► SET ClimbDirection
│   │   │   │   │   │
│   │   │   │   │   └─► Update Helper Variables
│   │   │   │   │       ├─► SET bIsOnWall (ClimbingSurfaceType == 1)
│   │   │   │   │       └─► SET bIsOnCeiling (ClimbingSurfaceType == 2)
```

**Blueprint Code (Pseudocode):**

```cpp
Event_Blueprint_Update_Animation()
{
    Character = TryGetPawnOwner();
    if (Character)
    {
        ClimbingMovement = Character->GetComponentByClass(ZombieClimbingMovementComponent);
        if (IsValid(ClimbingMovement))
        {
            bIsClimbing = ClimbingMovement->IsClimbing();
            ClimbingSurfaceType = ClimbingMovement->GetCurrentSurfaceType();
            ClimbSpeed = ClimbingMovement->GetVelocity().Size();
            ClimbDirection = ClimbingMovement->GetCurrentSurfaceNormal();

            // Helper variables
            bIsOnWall = (ClimbingSurfaceType == 1);
            bIsOnCeiling = (ClimbingSurfaceType == 2);
        }
    }
}
```

---

## State Machine Configuration

### Step 4: Create State Machine

In the **AnimGraph** of your Animation BP:

1. **Add State Machine:**
   - Right-click → **State Machines** → **Add New State Machine**
   - Name: `ZombieLocomotion`

2. **Connect to Output Pose:**
   ```
   [State Machine: ZombieLocomotion] → [Output Pose]
   ```

### Step 5: Define States

Double-click the State Machine and add these states:

```
State Machine: ZombieLocomotion
│
├─► Entry (automatic)
│
├─► Idle/Walking (existing or new)
│   └─► Output: Blend Space or Animation Sequence
│
├─► Climbing_Wall
│   └─► Output: Wall climbing animation
│
├─► Climbing_Ceiling
│   └─► Output: Ceiling climbing animation (rotated)
│
├─► Falling (existing or new)
│   └─► Output: Fall animation
│
└─► Attack (existing)
    └─► Output: Attack montage
```

### Step 6: State Transitions

#### From Idle/Walking to Climbing_Wall

**Transition Rule:**

```blueprint
Transition Rule: Idle/Walking → Climbing_Wall
│
├─► Can Enter Transition (Automatic Transition)
│   │
│   └─► AND
│       ├─► bIsClimbing == true
│       └─► bIsOnWall == true
│
└─► Blend Settings:
    ├─► Duration: 0.2 seconds
    └─► Blend Type: Cubic
```

#### From Climbing_Wall to Idle/Walking

**Transition Rule:**

```blueprint
Transition Rule: Climbing_Wall → Idle/Walking
│
├─► Can Enter Transition
│   │
│   └─► bIsClimbing == false
│
└─► Blend Settings:
    ├─► Duration: 0.3 seconds
    └─► Blend Type: Cubic
```

#### From Climbing_Wall to Climbing_Ceiling

**Transition Rule:**

```blueprint
Transition Rule: Climbing_Wall → Climbing_Ceiling
│
├─► Can Enter Transition
│   │
│   └─► AND
│       ├─► bIsClimbing == true
│       └─► bIsOnCeiling == true
│
└─► Blend Settings:
    ├─► Duration: 0.4 seconds (slower for smooth transition)
    └─► Blend Type: Cubic
```

#### From Climbing_Ceiling to Climbing_Wall

**Transition Rule:**

```blueprint
Transition Rule: Climbing_Ceiling → Climbing_Wall
│
├─► Can Enter Transition
│   │
│   └─► AND
│       ├─► bIsClimbing == true
│       └─► bIsOnWall == true
│
└─► Blend Settings:
    ├─► Duration: 0.4 seconds
    └─► Blend Type: Cubic
```

#### From Climbing to Falling (Drop Attack)

**Transition Rule:**

```blueprint
Transition Rule: Climbing_Wall/Ceiling → Falling
│
├─► Can Enter Transition
│   │
│   └─► AND
│       ├─► bIsClimbing == false
│       └─► Character->GetMovementMode() == Falling
│
└─► Blend Settings:
    ├─► Duration: 0.1 seconds (quick)
    └─► Blend Type: Linear
```

---

## Animation Blending

### Step 7: Configure State Animations

#### State: Climbing_Wall

```blueprint
State: Climbing_Wall
│
├─► Blend Poses by ClimbSpeed
│   │
│   ├─► Alpha: ClimbSpeed / MaxWallClimbSpeed (e.g., 150.0)
│   │
│   ├─► Pose A (ClimbSpeed = 0): Wall_Idle
│   │   └─► Animation: ZombieIdle (or first frame of ZombieWalking)
│   │
│   └─► Pose B (ClimbSpeed = max): Wall_Climb
│       └─► Animation: ZombieWalking (played vertically)
│
└─► Output Animation Pose
```

**Detailed Setup:**

1. In the **Climbing_Wall** state, add:
   - **Blend node:** Right-click → **Blend** → **Blend Poses by Float**

2. Configure blend:
   ```
   Alpha Pin: ClimbSpeed / 150.0 (or your MaxWallClimbSpeed)
   Pose A: Wall idle animation
   Pose B: Wall climbing cycle animation
   ```

3. Add **Layered Blend per Bone** (optional):
   - Blend upper body separately for attacks while climbing

#### State: Climbing_Ceiling

```blueprint
State: Climbing_Ceiling
│
├─► Play Animation: ZombieCrawl
│   Play Rate: ClimbSpeed / 120.0
│   Loop: true
│   │
│   └─► Transform (Modify) Bone
│       │
│       ├─► Bone: Root (or Pelvis)
│       ├─► Rotation Space: Mesh Space
│       └─► Rotation: (Roll: 180°, Pitch: 0°, Yaw: 0°)
│           (This flips zombie upside down)
│
└─► Output Animation Pose
```

**Alternative Method - Using Blend Space:**

```blueprint
State: Climbing_Ceiling
│
├─► BlendSpace: Zombie_Ceiling_BS
│   Horizontal Axis: ClimbDirection.X (-1 to 1)
│   Vertical Axis: ClimbDirection.Y (-1 to 1)
│   │
│   └─► Blend Space Contents:
│       ├─► Center: Ceiling_Idle
│       ├─► Up: Ceiling_Crawl_Forward
│       ├─► Down: Ceiling_Crawl_Backward
│       ├─► Left: Ceiling_Crawl_Left
│       └─► Right: Ceiling_Crawl_Right
│
└─► Output Animation Pose
```

### Step 8: Create Blend Spaces (Optional but Recommended)

#### Wall Climbing Blend Space

1. **Create 1D Blend Space:**
   - Right-click → **Animation** → **Blend Space 1D**
   - Name: `BS_WallClimb`
   - Skeleton: Zombie skeleton

2. **Configure Axis:**
   ```
   Horizontal Axis:
   ├─► Name: Speed
   ├─► Minimum: 0.0
   ├─► Maximum: 150.0
   └─► Divisions: 3
   ```

3. **Add Animation Samples:**
   ```
   Position 0.0: ZombieIdle (or wall idle pose)
   Position 75.0: ZombieWalking (50% speed)
   Position 150.0: ZombieRun (or fast climbing)
   ```

#### Ceiling Crawl Blend Space

1. **Create 2D Blend Space:**
   - Right-click → **Animation** → **Blend Space**
   - Name: `BS_CeilingCrawl`

2. **Configure Axes:**
   ```
   Horizontal Axis (X):
   ├─► Name: Direction_X
   ├─► Range: -1.0 to 1.0

   Vertical Axis (Y):
   ├─► Name: Direction_Y
   ├─► Range: -1.0 to 1.0
   ```

3. **Add Samples:**
   ```
   Center (0, 0): Ceiling_Idle
   Top (0, 1): ZombieCrawl_Forward
   Bottom (0, -1): ZombieCrawl_Backward
   Left (-1, 0): ZombieCrawl_Left (mirrored forward)
   Right (1, 0): ZombieCrawl_Right (mirrored forward)
   ```

---

## Retargeting Existing Animations

### Option 1: Reuse Walking Animation for Walls

Since you have `ZombieWalking` and `ZombieRun` animations, you can reuse them:

1. **In Climbing_Wall state:**
   - Use `ZombieWalking` directly
   - The character's rotation (handled by movement component) makes it appear vertical

2. **Adjust play rate based on speed:**
   ```blueprint
   Play Animation: ZombieWalking
   ├─► Play Rate: ClimbSpeed / 150.0
   └─► Loop: true
   ```

### Option 2: Reuse Crawl Animation for Ceiling

You have `ZombieCrawl` which is perfect for ceiling movement:

1. **In Climbing_Ceiling state:**
   ```blueprint
   Sequence Evaluator: ZombieCrawl
   ├─► Play Rate: ClimbSpeed / 120.0
   ├─► Loop: true
   │
   └─► Transform Bone (Root)
       └─► Rotation: (180°, 0°, 0°) - Flip upside down
   ```

2. **The crawl animation is already low to ground, perfect for hanging upside down**

---

## Creating New Climbing Animations

If you want to create custom climbing animations:

### Method 1: Using Blender (Recommended)

You already have Blender imports (`ZombieCrawlBlender.uasset`), so:

1. **Export Zombie Skeleton** from Unreal
2. **Open in Blender** and create keyframe animation:
   - **Wall Climb:** Alternating hand-over-hand movement
   - **Ceiling Crawl:** Spider-like limb movement
3. **Export as FBX** and import back to Unreal
4. **Retarget** to zombie skeleton if needed

### Method 2: Animation Montages

Create montages for special climbing moves:

1. **Wall Attack Montage:**
   ```
   Content/HGT/AI_Zombie/Animations/
   └─► Create Animation Montage
       ├─► Base: Zombie_Attack
       ├─► Add Section: "WallSwipe"
       └─► Play in Climbing_Wall state when attacking
   ```

2. **Drop Attack Montage:**
   ```
   Create Animation Montage
   ├─► Base: ZombieCrawlAttack
   ├─► Add Section: "DropPounce"
   └─► Play when ExecuteDropAttack is called
   ```

### Method 3: Procedural Animation (Advanced)

Use **Animation Blueprint** procedural nodes:

```blueprint
In Climbing_Wall State:
│
├─► Two Bone IK (Left Hand)
│   Target: Trace hit location on wall
│
├─► Two Bone IK (Right Hand)
│   Target: Trace hit location on wall
│
├─► Two Bone IK (Left Foot)
│   Target: Trace hit location on wall
│
└─► Two Bone IK (Right Foot)
    Target: Trace hit location on wall
```

This makes limbs stick to irregular surfaces.

---

## Testing and Debugging

### Step 9: Test Animations in Editor

1. **Open Animation Blueprint**
2. **Preview Tab** → Set variables manually:
   ```
   bIsClimbing: true
   ClimbingSurfaceType: 1 (Wall)
   ClimbSpeed: 100.0
   ```

3. **Watch State Machine:**
   - Should transition to Climbing_Wall state
   - Animation should play

4. **Test Transitions:**
   ```
   Set bIsClimbing: false → Should return to Idle
   Set ClimbingSurfaceType: 2 → Should go to Ceiling
   ```

### Step 10: Test In-Game

1. **Place zombie in level**
2. **Enable debug:**
   ```
   In Zombie BP, Set:
   ├─► ZombieClimbingMovement->bShowDebugTraces = true
   ├─► ClimbingAI->bShowDebugInfo = true
   ```

3. **Watch for:**
   - ✓ Smooth transitions between states
   - ✓ Animation speed matches movement speed
   - ✓ Correct orientation (especially on ceiling)
   - ✗ Foot sliding or skating
   - ✗ Sudden rotation snaps

### Common Animation Issues

#### Issue: Feet sliding on wall

**Solution:**
- Increase `RotationSpeed` on movement component
- Add IK to feet in Animation BP
- Adjust animation root motion

#### Issue: Jerky transitions

**Solution:**
- Increase transition blend duration
- Use **Inertialization** blend mode
- Add transition animations

#### Issue: Wrong orientation on ceiling

**Solution:**
- Check Transform Bone rotation (should be 180° roll)
- Verify `UpdateClimbingRotation` is working
- Adjust in Animation BP, not movement code

#### Issue: Animation doesn't match speed

**Solution:**
- Verify `ClimbSpeed` variable updates correctly
- Adjust blend space axis ranges
- Match animation play rate to actual movement speed

---

## Advanced: Animation Notifies

### Adding Attack Notifies While Climbing

1. **Open** `Zombie_Attack_Montage` (or create new montage)

2. **Add Anim Notify:**
   - Timeline position: Attack contact frame
   - Notify Class: `AnimNotify_ClimbingAttack` (create custom)

3. **In Climbing State, play montage:**
   ```blueprint
   Event: OnAttackInput (from AI or gameplay)
   │
   ├─► Is Climbing? (Check)
   │   │
   │   ├─► [TRUE] Play Montage: Wall_Attack_Montage
   │   └─► [FALSE] Play Montage: Normal_Attack_Montage
   ```

### Drop Attack Animation Notify

Create a notify for the drop attack to time the damage:

```blueprint
AnimNotify_DropAttackImpact
│
├─► Triggered at: Landing frame of drop animation
│
└─► Notify Implementation:
    └─► Sphere Overlap at zombie location
        ├─► Radius: 150 units
        └─► Damage nearby players
```

---

## Animation Blueprint Summary

**Final Animation BP Structure:**

```
AnimBP_ZombieClimbing
│
├─► Event Graph
│   └─► Update climbing variables from component
│
├─► AnimGraph
│   └─► State Machine: ZombieLocomotion
│       │
│       ├─► Idle/Walking
│       │   └─► Existing locomotion blend space
│       │
│       ├─► Climbing_Wall
│       │   └─► Blend (Idle ↔ Walking) by ClimbSpeed
│       │
│       ├─► Climbing_Ceiling
│       │   └─► ZombieCrawl (rotated 180°)
│       │
│       ├─► Falling
│       │   └─► Fall animation
│       │
│       └─► Attack
│           └─► Attack montage
│
└─► Variables
    ├─► bIsClimbing
    ├─► ClimbingSurfaceType
    ├─► ClimbSpeed
    ├─► bIsOnWall
    └─► bIsOnCeiling
```

---

## Quick Reference: Animation Checklist

- [ ] Animation Blueprint created or duplicated
- [ ] Variables added (bIsClimbing, ClimbSpeed, etc.)
- [ ] Event Graph updates variables from movement component
- [ ] State Machine has Climbing_Wall state
- [ ] State Machine has Climbing_Ceiling state
- [ ] Transitions configured with proper conditions
- [ ] Wall climbing animation assigned (or reused Walking)
- [ ] Ceiling animation assigned (or reused Crawl + rotation)
- [ ] Blend spaces created (optional but recommended)
- [ ] Tested in Animation Blueprint preview
- [ ] Tested in-game with debug enabled
- [ ] Transitions are smooth
- [ ] No foot sliding or skating
- [ ] Ceiling orientation correct (upside down)
- [ ] Attack animations work while climbing (optional)

---

## Next Steps

1. **Set up basic state machine** with existing animations
2. **Test climbing movement** to ensure states trigger correctly
3. **Refine transitions** for smoothness
4. **Create custom animations** if needed
5. **Add attack montages** for climbing combat
6. **Polish with IK** and procedural animation

---

**Animation setup complete! Your zombies should now have smooth climbing animations.** 🧟‍♂️🎬
