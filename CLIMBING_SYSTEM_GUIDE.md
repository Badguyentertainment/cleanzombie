# Zombie Wall & Ceiling Climbing System - Integration Guide

## Overview

This guide explains how to integrate the wall and ceiling climbing system into your zombie characters in the Unreal Engine Multiplayer Zombie Template.

## Table of Contents

1. [System Architecture](#system-architecture)
2. [Quick Start](#quick-start)
3. [Blueprint Integration](#blueprint-integration)
4. [Animation Setup](#animation-setup)
5. [AI Configuration](#ai-configuration)
6. [Multiplayer Considerations](#multiplayer-considerations)
7. [Level Design](#level-design)
8. [Customization](#customization)
9. [Troubleshooting](#troubleshooting)

---

## System Architecture

The climbing system consists of three main C++ components:

### 1. **ZombieClimbingMovementComponent**
- Custom Character Movement Component
- Handles physics and movement for wall/ceiling climbing
- Manages surface detection and attachment
- Fully replicated for multiplayer

### 2. **ClimbingAIComponent**
- AI decision-making for climbing behavior
- Determines when and where to climb
- Handles drop attacks from surfaces
- Integrates with existing zombie AI

### 3. **ClimbableZoneActor**
- Level design helper actor
- Marks specific areas as climbable
- Configurable climb speed modifiers
- AI pathfinding integration

---

## Quick Start

### Step 1: Compile C++ Code

1. **Close Unreal Editor** if it's currently open
2. **Right-click** on `cleanzombie.uproject` → **Generate Visual Studio project files**
3. **Open** the generated Visual Studio solution
4. **Build** the project in Development Editor configuration
5. **Launch** Unreal Editor from Visual Studio or the .uproject file

### Step 2: Update Your Zombie Blueprint

1. Open your zombie Blueprint (e.g., `Content/HGT/AI_Zombie/BaseZombie`)
2. **Replace the CharacterMovementComponent**:
   - Select the CharacterMovement component
   - In Details panel, change **Component Class** to `ZombieClimbingMovementComponent`
3. **Add the ClimbingAIComponent**:
   - Click **Add Component** → Search for `Climbing AI Component`
   - Add it to your zombie Blueprint

### Step 3: Configure Climbing Parameters

Select the **ZombieClimbingMovementComponent** and configure:

```
Speed Settings:
├── Max Wall Climb Speed: 150.0
├── Max Ceiling Climb Speed: 120.0
├── Climbing Acceleration: 500.0
└── Climbing Deceleration: 1000.0

Detection Settings:
├── Climbable Detection Distance: 100.0
├── Climbable Detection Radius: 30.0
├── Wall Min Angle: 60.0
└── Ceiling Max Angle: 135.0

Abilities:
├── ☑ Climbing Enabled
├── ☑ Can Drop From Surfaces
└── ☑ Auto Transition Between Surfaces
```

---

## Blueprint Integration

### A. Using Climbing in Blueprint Events

#### Check if Zombie is Climbing

```blueprint
Event Graph:
┌─────────────────────────────┐
│ Event Tick                  │
└─────────┬───────────────────┘
          │
┌─────────▼─────────────────────┐
│ Get Component By Class        │
│ (ZombieClimbingMovement)      │
└─────────┬─────────────────────┘
          │
┌─────────▼─────────────────────┐
│ Is Climbing (Pure Function)   │
└─────────┬─────────────────────┘
          │
┌─────────▼─────────────────────┐
│ Branch                        │
│ True: Play Climbing Animation │
│ False: Play Normal Animation  │
└───────────────────────────────┘
```

#### Manually Start Climbing

```blueprint
Custom Event: "StartClimbing"
┌─────────────────────────────┐
│ Get Component By Class      │
│ (ZombieClimbingMovement)    │
└─────────┬───────────────────┘
          │
┌─────────▼─────────────────────┐
│ Try Start Climbing            │
│ Return: Bool (Success)        │
└───────────────────────────────┘
```

#### Stop Climbing

```blueprint
Custom Event: "StopClimbing"
┌─────────────────────────────┐
│ Get Component By Class      │
│ (ZombieClimbingMovement)    │
└─────────┬───────────────────┘
          │
┌─────────▼─────────────────────┐
│ Stop Climbing                 │
└───────────────────────────────┘
```

#### Drop Attack

```blueprint
Custom Event: "DropOnTarget"
┌─────────────────────────────┐
│ Get Component By Class      │
│ (ClimbingAIComponent)       │
└─────────┬───────────────────┘
          │
┌─────────▼─────────────────────┐
│ Execute Drop Attack           │
└───────────────────────────────┘
```

### B. AI Integration

#### Setting AI Target

```blueprint
Event Graph (AI Controller or Zombie BP):
┌─────────────────────────────┐
│ Get Player Character        │
└─────────┬───────────────────┘
          │
┌─────────▼─────────────────────┐
│ Get Component By Class        │
│ Owner: Self                   │
│ (ClimbingAIComponent)         │
└─────────┬─────────────────────┘
          │
┌─────────▼─────────────────────┐
│ Set Target                    │
│ New Target: Player Character  │
└───────────────────────────────┘
```

#### Configure AI Behavior

Select the **ClimbingAIComponent** in your zombie Blueprint:

```
Behavior Settings:
├── ☑ Auto Climbing (AI decides when to climb)
├── Climbing Check Interval: 0.5
├── ☑ Climb When Path Blocked
├── Climbing Consideration Distance: 500.0
└── ☑ Prefer Climbing Paths (optional)

Attack Settings:
├── ☑ Drop To Attack
├── Drop Attack Distance: 300.0
├── Drop Attack Height Min: 100.0
├── Max Drop Height: 1000.0
└── Minimum Climb Time: 2.0

Debug:
└── ☑ Show Debug Info (for testing)
```

---

## Animation Setup

### A. Animation Blueprint Integration

You'll need to add climbing states to your zombie Animation Blueprint.

#### 1. Create Animation State Machine States

Open your zombie Animation Blueprint (or create a new one):

**States Needed:**
- **Climbing Wall** - For vertical wall climbing
- **Climbing Ceiling** - For upside-down ceiling movement
- **Climbing Idle** - When attached but not moving
- **Drop From Surface** - Transition to falling

#### 2. State Transitions

```
State Machine Flow:
┌──────────┐     IsClimbing == true     ┌──────────────┐
│  Idle/   │─────────────────────────→  │  Climbing    │
│  Walking │                             │  Wall        │
└──────────┘     IsClimbing == false    └──────────────┘
      ↑          ←─────────────────────         │
      │                                          │
      │          SurfaceType == Ceiling         │
      │          ←───────────────────────────┐  │
      │                                       │  │
      └───────────────────────────────────┐  │  │
                                           │  │  │
                                    ┌──────▼──▼──▼──────┐
                                    │  Climbing Ceiling  │
                                    └────────────────────┘
```

#### 3. Animation Blueprint Variables

Create these variables in your Animation Blueprint:

| Variable Name | Type | Description |
|---------------|------|-------------|
| `IsClimbing` | Boolean | Is character currently climbing? |
| `ClimbSpeed` | Float | Current climbing speed |
| `SurfaceType` | Enum (EClimbingSurfaceType) | Wall, Ceiling, or None |
| `ClimbDirection` | Vector | Direction of climbing movement |

#### 4. Update Animation Variables (Event Graph)

```blueprint
Event Blueprint Update Animation:
┌─────────────────────────────────┐
│ Try Get Pawn Owner              │
└─────────┬───────────────────────┘
          │
┌─────────▼─────────────────────────┐
│ Cast to Character                 │
└─────────┬─────────────────────────┘
          │
┌─────────▼─────────────────────────────────┐
│ Get Component By Class                    │
│ (ZombieClimbingMovementComponent)         │
└─────────┬─────────────────────────────────┘
          │
┌─────────▼─────────────────────────┐
│ Is Climbing → Set IsClimbing      │
└─────────┬─────────────────────────┘
          │
┌─────────▼──────────────────────────────┐
│ Get Current Surface Type               │
│ → Set SurfaceType                      │
└─────────┬──────────────────────────────┘
          │
┌─────────▼──────────────────────────────┐
│ Get Current Surface Normal             │
│ → Set ClimbDirection                   │
└────────────────────────────────────────┘
```

### B. Suggested Animations

You'll need these animation types:

1. **Wall Climbing Cycle** - Forward movement on walls
   - Similar to crawling but vertical
   - Can reuse/modify existing `ZombieWalking` animation

2. **Ceiling Crawl Cycle** - Upside-down movement
   - Can repurpose existing `ZombieCrawl` animation
   - Rotated 180 degrees in the state

3. **Wall Idle** - Clinging to wall without moving
   - Static pose, arms extended
   - Can be first frame of wall climb cycle

4. **Ceiling Idle** - Hanging from ceiling
   - Static upside-down pose

5. **Surface Transition** - Blending between surfaces
   - Use blend spaces for smooth transitions

### C. Example Animation State Implementation

**Climbing Wall State:**

```blueprint
State: Climbing_Wall
├── Output Animation Pose
│   └── Play Animation: ZombieWalking (or custom wall climb)
├── Blend by ClimbSpeed
│   ├── Alpha: ClimbSpeed / MaxWallClimbSpeed
│   ├── Animation A: Wall_Idle
│   └── Animation B: Wall_Climb_Cycle
```

**Climbing Ceiling State:**

```blueprint
State: Climbing_Ceiling
├── Output Animation Pose
│   └── Play Animation: ZombieCrawl
├── Modify Bone: Root
│   └── Rotation: (0, 0, 180) - Flip upside down
├── Blend by ClimbSpeed
│   ├── Alpha: ClimbSpeed / MaxCeilingClimbSpeed
│   ├── Animation A: Ceiling_Idle
│   └── Animation B: Ceiling_Crawl_Cycle
```

---

## AI Configuration

### A. Basic AI Setup

For your zombie AI Controller Blueprint:

#### 1. Initialize AI Target

```blueprint
Event Begin Play:
┌─────────────────────────────┐
│ Get Player Character        │
└─────────┬───────────────────┘
          │
┌─────────▼─────────────────────────┐
│ Get Controlled Pawn               │
└─────────┬─────────────────────────┘
          │
┌─────────▼─────────────────────────────┐
│ Get Component By Class                │
│ (ClimbingAIComponent)                 │
└─────────┬─────────────────────────────┘
          │
┌─────────▼─────────────────────────┐
│ Set Target (Player Character)     │
└───────────────────────────────────┘
```

#### 2. Behavior Tree Integration (Optional)

If using Behavior Trees, create these tasks:

**BTTask_TryStartClimbing:**
- Checks if climbing is beneficial
- Calls `TryStartClimbing` on movement component
- Returns Success/Failure

**BTTask_UpdateClimbingMovement:**
- Sets climb direction toward target
- Updates while zombie is climbing
- Returns Success when target reached

**BTTask_DropAttack:**
- Checks if should drop on target
- Calls `ExecuteDropAttack`
- Returns Success

### B. Advanced AI Behaviors

#### Dynamic Target Selection

```blueprint
Custom Event: "FindBestTarget"
┌──────────────────────────────┐
│ Get All Actors Of Class      │
│ (Player Character)           │
└─────────┬────────────────────┘
          │
┌─────────▼────────────────────────┐
│ For Each Loop                    │
│ ├── Get Distance To              │
│ ├── Store if Closest             │
│ └── Set as New Target            │
└──────────────────────────────────┘
```

#### Climbing Decision Logic

```blueprint
Custom Function: "ShouldAttemptClimb"
┌──────────────────────────────────┐
│ Get ClimbingAIComponent          │
└─────────┬────────────────────────┘
          │
┌─────────▼────────────────────────┐
│ Should Climb To Target           │
│ Return: Boolean                  │
└──────────────────────────────────┘
```

---

## Multiplayer Considerations

### A. Replication

The climbing system is **fully replicated** and multiplayer-ready:

- **Movement State** - Automatically replicated via `ClimbingState`
- **Surface Normal** - Replicated for rotation sync
- **AI Decisions** - Server-authoritative

### B. Server Authority

Climbing logic runs on the **server** for AI zombies:
- Clients receive replicated movement updates
- Smooth interpolation handled by CharacterMovementComponent
- No additional work needed

### C. Testing Multiplayer

1. **Enable Debug Visualization:**
   ```
   ZombieClimbingMovementComponent:
   ☑ Show Debug Traces

   ClimbingAIComponent:
   ☑ Show Debug Info
   ```

2. **Test in PIE (Play in Editor):**
   - Set Number of Players: 2+
   - Set Net Mode: Play As Listen Server
   - Watch for climbing sync between server/client

3. **Common Issues:**
   - **Jittery Movement:** Increase tick rate or adjust interpolation
   - **Desync:** Check network settings and replication
   - **AI not climbing:** Verify AI component is initialized on server

---

## Level Design

### A. Using ClimbableZoneActor

1. **Place in Level:**
   - Drag `ClimbableZoneActor` from Content Browser into level
   - Position and scale to cover desired climbable area

2. **Configure Zone:**
   ```
   ClimbableZoneActor Properties:
   ├── ☑ Is Active
   ├── Climb Speed Multiplier: 1.0 (normal), 0.5 (slippery), 1.5 (fast)
   ├── ☑ AI Climbable
   └── ☑ Show Debug (for visualization in-game)
   ```

3. **Volume Sizing:**
   - Extend box to cover entire climbable surface
   - Include some buffer space around edges
   - Multiple zones can overlap

### B. World Geometry Climbing

For climbing on **any world geometry** (without ClimbableZoneActor):

1. The system uses **trace channels** to detect surfaces
2. Configure in `ZombieClimbingMovementComponent`:
   ```
   Climbable Trace Channel: Visibility (default)
   ```

3. **To make specific actors climbable:**
   - Set collision response to `Visibility` channel: **Block**
   - Surface normal must meet angle requirements

### C. Level Design Tips

**Good Climbing Surfaces:**
- Walls at 60-90° from horizontal
- Ceilings at 135-180° from horizontal
- Connected surfaces for transitions
- Areas above player paths (for drop attacks)

**Avoid:**
- Very steep slopes (may be detected as walls)
- Extremely thin geometry (can cause clipping)
- Rapidly changing surface normals

**Strategic Placement:**
- Create flanking routes via walls/ceilings
- Design vertical ambush points
- Connect multi-level areas with climbing paths
- Balance with player sight lines

---

## Customization

### A. Adjusting Climb Speeds

Different zombie types can have different climb speeds:

```blueprint
Event Begin Play (Zombie BP):
┌─────────────────────────────────┐
│ Get ZombieClimbingMovement      │
└─────────┬───────────────────────┘
          │
┌─────────▼─────────────────────────┐
│ Set Max Wall Climb Speed          │
│ Value: 200.0 (Fast Zombie)        │
│        150.0 (Normal Zombie)      │
│        100.0 (Slow Zombie)        │
└───────────────────────────────────┘
```

### B. Custom Surface Detection

Implement `ClimbableSurfaceInterface` in Blueprint:

1. **Create Blueprint** implementing `ClimbableSurfaceInterface`
2. **Implement functions:**
   - `Can Be Climbed` - Return true/false based on custom logic
   - `Get Climb Speed Multiplier` - Return speed modifier
   - `On Climbing Started` - Custom behavior when climbed
   - `On Climbing Stopped` - Cleanup when zombie leaves

### C. Special Abilities

#### Electric Shock on Ceiling

```blueprint
Ceiling Climbing State:
┌──────────────────────────────┐
│ Get Current Surface Type     │
└─────────┬────────────────────┘
          │
┌─────────▼────────────────────────┐
│ Branch (if Ceiling)              │
│ True → Spawn Electric Effect     │
│ False → Do Nothing               │
└──────────────────────────────────┘
```

#### Limited Climb Stamina

```blueprint
Event Tick:
┌──────────────────────────────┐
│ Get ClimbingAIComponent      │
└─────────┬────────────────────┘
          │
┌─────────▼────────────────────────────┐
│ Get Current Climb Time               │
└─────────┬────────────────────────────┘
          │
┌─────────▼────────────────────────┐
│ Branch (if > Max Climb Time)     │
│ True → Force Stop Climbing       │
└──────────────────────────────────┘
```

---

## Troubleshooting

### Common Issues

#### 1. Zombie Won't Start Climbing

**Symptoms:** Zombie walks to wall but doesn't climb

**Solutions:**
- ✓ Verify `bClimbingEnabled = true` on movement component
- ✓ Check surface angle meets `WallMinAngle` requirements
- ✓ Increase `ClimbableDetectionDistance`
- ✓ Enable `bShowDebugTraces` to visualize detection
- ✓ Ensure collision channel is set correctly

#### 2. Jittery Climbing Movement

**Symptoms:** Zombie shakes or vibrates while climbing

**Solutions:**
- ✓ Reduce `RotationSpeed` (try 5.0 instead of 10.0)
- ✓ Increase `SurfaceOffset` slightly
- ✓ Check for overlapping geometry
- ✓ Smooth out surface normals in the level

#### 3. Falls Off Surface

**Symptoms:** Zombie starts climbing but falls off immediately

**Solutions:**
- ✓ Surface type changed (check angle thresholds)
- ✓ Increase `SurfaceValidationInterval` checks
- ✓ Disable `bAutoTransitionBetweenSurfaces` if transitioning incorrectly
- ✓ Check for gaps in geometry

#### 4. AI Doesn't Climb Automatically

**Symptoms:** Manual climbing works, but AI won't climb

**Solutions:**
- ✓ Verify `bAutoClimbing = true` on ClimbingAIComponent
- ✓ Call `SetTarget` to give AI a target to pursue
- ✓ Check `ClimbingConsiderationDistance` is large enough
- ✓ Enable `bShowDebugInfo` to see AI decision making

#### 5. Multiplayer Desync

**Symptoms:** Climbing looks different on client vs server

**Solutions:**
- ✓ Verify component is set to replicate
- ✓ Check network conditions (lag, packet loss)
- ✓ Ensure CharacterMovementComponent networking is enabled
- ✓ Test with lower movement speeds

#### 6. Wrong Orientation on Ceiling

**Symptoms:** Zombie is sideways or wrong rotation on ceiling

**Solutions:**
- ✓ Check `CeilingMaxAngle` threshold
- ✓ Verify `RotationSpeed` isn't too fast or slow
- ✓ Adjust surface normal calculation
- ✓ Check for conflicting rotation logic in Animation BP

---

## Debug Commands

### Console Commands (for testing)

Enable these in your zombie Blueprint for testing:

```blueprint
Keyboard Event "C":
└── Try Start Climbing

Keyboard Event "X":
└── Stop Climbing

Keyboard Event "V":
└── Drop From Surface

Keyboard Event "B":
└── Toggle bShowDebugTraces
```

### Visual Debugging

Enable visual debug:
```
ZombieClimbingMovementComponent:
☑ Show Debug Traces

ClimbingAIComponent:
☑ Show Debug Info
```

**What you'll see:**
- **Green lines** - Successful surface detection
- **Red lines** - Failed traces
- **Cyan spheres** - Trace start/end points
- **Blue arrows** - Surface normals
- **Orange arrows** - Climb direction
- **Yellow text** - Current surface type and AI state

---

## Performance Optimization

### For Many Climbing Zombies

1. **Reduce Trace Frequency:**
   ```cpp
   SurfaceValidationInterval = 0.2f; // Default 0.1f
   ClimbingCheckInterval = 1.0f; // Default 0.5f
   ```

2. **Use Simpler Traces:**
   ```cpp
   ClimbableDetectionRadius = 0.0f; // Line trace instead of sphere
   ```

3. **Limit Climbing Zombies:**
   ```blueprint
   Only allow X zombies to climb simultaneously
   Others wait their turn or use ground paths
   ```

4. **LOD for Distant Climbers:**
   ```blueprint
   Disable debug visualization when far from player
   Reduce tick frequency for distant climbing AI
   ```

---

## Next Steps

1. **Compile the C++ code**
2. **Update your zombie Blueprint** with the new components
3. **Create climbing animations** or adapt existing ones
4. **Test in single player** with debug enabled
5. **Test in multiplayer** to verify replication
6. **Place ClimbableZoneActors** in your levels
7. **Configure AI behavior** to your desired aggressiveness
8. **Polish animations and transitions**

---

## Support & Additional Resources

- **Component Reference:** See header files in `Source/cleanzombie/Public/`
- **Example Usage:** Check function comments for Blueprint-callable functions
- **Unreal Documentation:** [Character Movement Component](https://docs.unrealengine.com/en-US/Gameplay/Framework/Pawn/Character/)

---

## Credits

**Climbing System v1.0**
- Custom Character Movement Component
- AI Climbing Behavior System
- Multiplayer-ready implementation
- Designed for Unreal Engine 5.6+

---

**Happy Climbing! 🧟‍♂️🧗**
