# 🚇 ZOMBIE TUNNEL NAVIGATION SYSTEM - Complete Guide

**Version:** 1.0
**Engine:** Unreal Engine 5.6
**Template:** Multiplayer Zombie Template
**AI System:** AI Behavior Toolkit

---

## 📋 Table of Contents

1. [System Overview](#system-overview)
2. [Architecture](#architecture)
3. [Quick Start (5 Minutes)](#quick-start)
4. [Component Reference](#component-reference)
5. [Level Setup](#level-setup)
6. [AI Integration](#ai-integration)
7. [Blueprint Setup](#blueprint-setup)
8. [Multiplayer Replication](#multiplayer-replication)
9. [Advanced Usage](#advanced-usage)
10. [Troubleshooting](#troubleshooting)
11. [Performance Optimization](#performance-optimization)

---

## 🎯 System Overview

The Tunnel Navigation System allows zombies to traverse pipes, vents, and tunnel networks as alternative pathfinding routes to surprise players.

### Key Features

✅ **Spline-Based Navigation** - Smooth movement along custom tunnel paths
✅ **Multiple Entry/Exit Points** - Zombies can enter and exit at any configured point
✅ **AI Pathfinding Integration** - NavLink system makes tunnels valid pathfinding options
✅ **Automatic Collision Modification** - Zombies shrink to fit tunnel size
✅ **Multiplayer Ready** - Full replication support
✅ **Modular Ability System** - Integrates with existing zombie framework
✅ **Trigger-Based Detection** - Automatic tunnel entrance/exit detection
✅ **Configurable Tunnel Sizes** - Small, Medium, Large presets

### How It Works

```
Player Running → Zombie Seeks → Detects Tunnel Nearby → Evaluates Path
    ↓                                                           ↓
Zombie Emerges ← Exits Tunnel ← Traverses Spline ← Enters Tunnel
```

---

## 🏗️ Architecture

### Core Components

**1. ATunnelVolume** (`TunnelVolume.h/cpp`)
- Actor placed in level representing tunnel system
- Contains spline defining tunnel path
- Manages entry/exit trigger volumes
- Tracks actors currently in tunnel
- Provides query functions for pathfinding

**2. UTunnelNavigationAbility** (`TunnelNavigationAbility.h/cpp`)
- Zombie ability component for tunnel traversal
- Handles entry/exit state machine
- Modifies zombie collision and scale
- Moves zombie along spline
- Integrates with AI pathfinding

**3. ATunnelNavLinkProxy** (`TunnelNavLinkProxy.h/cpp`)
- Creates smart nav links for AI pathfinding
- Connects tunnel entrances to exits
- Allows AI to consider tunnels in path planning
- Triggers TunnelNavigationAbility when used

### Data Flow

```cpp
// 1. Zombie seeks target
AI Behavior Toolkit Seek State
    ↓
// 2. Pathfinding considers tunnel
NavigationSystem finds path through TunnelNavLinkProxy
    ↓
// 3. Smart link triggers ability
ATunnelNavLinkProxy::OnSmartLinkReceived()
    ↓
// 4. Zombie enters tunnel
UTunnelNavigationAbility::EnterTunnel()
    ↓
// 5. Zombie traverses spline
UpdateSplineMovement() - moves along path
    ↓
// 6. Zombie exits at best exit point
ExitTunnel() - restores normal state
```

---

## ⚡ Quick Start

### Step 1: Add Tunnel Navigation to Zombie (5 minutes)

**In your Zombie Blueprint:**

1. Open `BP_Zombie` or your zombie variant Blueprint
2. Add Component → **Tunnel Navigation Ability**
3. Configure settings:
   ```
   Tunnel Movement Speed: 200
   bAuto Detect Tunnels: True
   bUse Tunnels For Pathfinding: True
   Tunnel Detection Radius: 1000
   ```

**Or in C++ (ZombieBase constructor):**
```cpp
// Add to ZombieBase.h
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
UTunnelNavigationAbility* TunnelNavigation;

// Add to ZombieBase.cpp constructor
TunnelNavigation = CreateDefaultSubobject<UTunnelNavigationAbility>(TEXT("TunnelNavigation"));
```

### Step 2: Create Tunnel Volume in Level (2 minutes)

1. **Place Actor:** Content Drawer → All → C++ Classes → cleanzombie → TunnelVolume
2. **Drag into level** at vent/pipe location
3. **Edit Spline:**
   - Select TunnelVolume
   - Click spline points in viewport
   - Alt+Drag to add points
   - Shape tunnel path through geometry
4. **Auto-Generate Entry Points:**
   - Details panel → Tunnel|Editor section
   - Click "Auto Generate Entry Points"
5. **Compile:** Click "Rebuild Trigger Volumes"

### Step 3: Create Nav Link (1 minute)

1. **Place Actor:** Content Drawer → TunnelNavLinkProxy
2. **Configure:**
   - Linked Tunnel: Select your TunnelVolume
   - bAuto Create Links: True
   - bBidirectional: True
3. **Done!** Nav links auto-created

### Testing

1. PIE (Play In Editor)
2. Spawn zombie near tunnel entrance
3. Make noise/get spotted far from tunnel exit
4. Watch zombie use tunnel as shortcut!

---

## 📦 Component Reference

### ATunnelVolume

#### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `TunnelSize` | ETunnelSize | Medium | Size category (Small/Medium/Large) |
| `TunnelSpeedMultiplier` | float | 0.7 | Speed multiplier in tunnel |
| `TunnelCapsuleRadius` | float | 30.0 | Zombie capsule radius in tunnel |
| `TunnelCapsuleHalfHeight` | float | 40.0 | Zombie capsule half height in tunnel |
| `ZombieScaleInTunnel` | float | 0.8 | Mesh scale factor in tunnel |
| `bAutoCreateTriggers` | bool | true | Auto-create entry triggers |
| `TriggerBoxExtent` | FVector | (100,100,100) | Trigger volume size |
| `TunnelNetworkID` | FName | - | Network ID for grouped tunnels |
| `bZombiesCanUse` | bool | true | Allow zombie usage |
| `bPlayersCanUse` | bool | false | Allow player usage |
| `PathfindingPriority` | float | 1.0 | Priority for AI pathfinding |

#### Functions

```cpp
// Query Functions
bool CanActorEnter(AActor* Actor, int32 EntryPointIndex);
int32 GetClosestEntryPoint(const FVector& Location, bool bEntrancesOnly);
FTunnelEntryPoint GetEntryPoint(int32 Index);
TArray<FTunnelEntryPoint> GetEntrancePoints();
TArray<FTunnelEntryPoint> GetExitPoints();

// Navigation
float GetTunnelLength(int32 EntryIndex, int32 ExitIndex);
FVector GetLocationAtDistance(float Distance);
FRotator GetRotationAtDistance(float Distance);
FTransform GetTransformAtDistance(float Distance);
int32 FindBestExitPoint(const FVector& TargetLocation);

// Editor Utilities
void RebuildTriggerVolumes();
void AddEntryPointAtSplinePoint(int32 SplinePointIndex);
void AutoGenerateEntryPoints();
```

#### Events (Blueprint Override)

```cpp
Event OnActorEnteredTunnel(Actor, EntryPointIndex)
Event OnActorExitedTunnel(Actor, ExitPointIndex)
Event OnActorTraversingTunnel(Actor, DistanceAlongSpline)
```

### UTunnelNavigationAbility

#### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `TunnelMovementSpeed` | float | 200.0 | Speed while traversing |
| `EnterDuration` | float | 0.5 | Entry animation time |
| `ExitDuration` | float | 0.5 | Exit animation time |
| `bRestoreCollisionOnExit` | bool | true | Restore original collision |
| `bRestoreMeshScaleOnExit` | bool | true | Restore original scale |
| `bAutoDetectTunnels` | bool | true | Auto-find nearby tunnels |
| `TunnelDetectionRadius` | float | 1000.0 | Detection range |
| `bUseTunnelsForPathfinding` | bool | true | Consider in pathfinding |
| `bPreferTunnels` | bool | false | Prefer over normal paths |
| `MaxTunnelDetourDistance` | float | 500.0 | Max detour allowed |

#### Functions

```cpp
// Navigation
bool EnterTunnel(ATunnelVolume* Tunnel, int32 EntryPointIndex, const FVector& TargetLocation);
void ExitTunnel();
void AbortTunnelTraversal();

// Queries
bool IsInTunnel();
bool IsTraversingTunnel();
ATunnelVolume* GetCurrentTunnel();
float GetTraversalProgress(); // 0-1

// Finding
ATunnelVolume* FindNearestTunnel(float MaxDistance, int32& OutEntryPointIndex);
bool CanUseTunnel(ATunnelVolume* Tunnel);
ATunnelVolume* FindBestTunnelToTarget(const FVector& TargetLocation, ...);
```

#### Events (Blueprint Override)

```cpp
Event OnTunnelEntered(Tunnel, EntryPointIndex)
Event OnTunnelExited(Tunnel, ExitPointIndex)
Event OnTunnelTraversing(Progress)
Event OnTunnelAborted()
```

### ATunnelNavLinkProxy

#### Properties

| Property | Type | Default | Description |
|----------|------|---------|-------------|
| `LinkedTunnel` | ATunnelVolume* | nullptr | Tunnel to create links for |
| `bAutoCreateLinks` | bool | true | Auto-create on spawn |
| `bBidirectional` | bool | true | Two-way links |
| `LinkCostMultiplier` | float | 1.0 | Pathfinding cost modifier |
| `bUseSmartLink` | bool | true | Use smart link system |

#### Functions

```cpp
void CreateTunnelNavLinks();
void ClearNavLinks();
void RefreshNavLinks();
int32 GetNumNavLinks();
```

---

## 🗺️ Level Setup

### Creating a Vent System

**Example: Ceiling Vent Network**

```
Office Room → Vent Entrance → Shaft → Junction → Multiple Exits
   ↓                                       ↓           ↓
Players                              Exit to         Exit to
Working                            Hallway        Storage Room
```

#### Step-by-Step:

**1. Build Vent Geometry**
- Create static mesh pipes/vents
- Make interiors walkable (collision)
- Place in ceiling/walls

**2. Add TunnelVolume**
```
Location: Inside vent mesh
Spline Points: Follow vent path
  Point 0: Entrance grate (office)
  Point 1: Vertical shaft
  Point 2: Horizontal run
  Point 3: Junction
  Point 4: Exit grate (hallway)
  Point 5: Exit grate (storage)
```

**3. Configure Entry Points**
```cpp
Entry Point 0:
  SplinePointIndex: 0
  bIsEntrance: true
  bIsExit: true
  EntryTag: "Office_Vent"

Entry Point 1:
  SplinePointIndex: 4
  bIsEntrance: false
  bIsExit: true
  EntryTag: "Hallway_Exit"

Entry Point 2:
  SplinePointIndex: 5
  bIsEntrance: false
  bIsExit: true
  EntryTag: "Storage_Exit"
```

**4. Tunnel Settings**
```cpp
TunnelSize: Small
TunnelSpeedMultiplier: 0.6
TunnelCapsuleRadius: 25.0
TunnelCapsuleHalfHeight: 30.0
ZombieScaleInTunnel: 0.7
PathfindingPriority: 1.5  // Prefer this route
```

**5. Add Nav Link**
- Place TunnelNavLinkProxy
- Set LinkedTunnel = your TunnelVolume
- Links auto-created!

### Multiple Tunnel Networks

**Create Network IDs:**
```cpp
TunnelVolume_VentSystem:
  TunnelNetworkID: "VentNetwork"

TunnelVolume_Pipes:
  TunnelNetworkID: "PipeNetwork"

TunnelVolume_Sewers:
  TunnelNetworkID: "SewerNetwork"
```

**Use in Blueprints:**
```cpp
// Find all tunnels in vent network
For Each (TunnelVolume in Level)
  If (TunnelNetworkID == "VentNetwork")
    Add to Array
```

---

## 🤖 AI Integration

### AI Behavior Toolkit Integration

The tunnel system integrates seamlessly with AI Behavior Toolkit's Seek behavior.

#### Automatic Integration

When zombie has TunnelNavigationAbility:
1. AI calculates path to target
2. NavigationSystem finds TunnelNavLinkProxy
3. Smart link triggers EnterTunnel()
4. Zombie automatically traverses
5. Exits near target
6. AI resumes normal behavior

**No additional setup required!**

#### Manual Tunnel Usage (Advanced)

**In Behavior Tree Task:**
```cpp
// Custom BTTask_UseTunnel

EBTNodeResult::Type UBTTask_UseTunnel::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController->GetPawn();

    // Get tunnel ability
    UTunnelNavigationAbility* TunnelAbility =
        ControlledPawn->FindComponentByClass<UTunnelNavigationAbility>();

    if (!TunnelAbility)
        return EBTNodeResult::Failed;

    // Get target location from blackboard
    FVector TargetLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector("TargetLocation");

    // Find best tunnel
    int32 EntryIndex, ExitIndex;
    ATunnelVolume* BestTunnel = TunnelAbility->FindBestTunnelToTarget(
        TargetLocation, 2000.0f, EntryIndex, ExitIndex);

    if (!BestTunnel)
        return EBTNodeResult::Failed;

    // Enter tunnel
    bool bSuccess = TunnelAbility->EnterTunnel(BestTunnel, EntryIndex, TargetLocation);

    return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
```

#### Blueprint Task Example

**BPTask_FindAndUseTunnel:**
```
Receive Execute AI:
  ├─ Get Controlled Pawn
  ├─ Get Component (TunnelNavigationAbility)
  ├─ Get Blackboard Value (Target Actor)
  ├─ Get Actor Location (Target)
  ├─ Find Best Tunnel To Target
  │    Max Search Radius: 3000
  │    Out Entry Index: [variable]
  │    Out Exit Index: [variable]
  ├─ Branch (Tunnel Valid?)
  │    True:
  │      ├─ Enter Tunnel
  │      └─ Finish Execute (Success)
  │    False:
  │      └─ Finish Execute (Failed)
```

---

## 🎨 Blueprint Setup

### Zombie Blueprint Configuration

**BP_Zombie_Tunneler** (example variant)

**1. Add Components:**
```
Components Panel:
  └─ TunnelNavigationAbility (Inherited)
       ├─ Tunnel Movement Speed: 250
       ├─ bAuto Detect Tunnels: true
       ├─ Tunnel Detection Radius: 1500
       └─ bPrefer Tunnels: true
```

**2. Override Events:**

**Event OnTunnelEntered:**
```cpp
Event On Tunnel Entered:
  ├─ Print String ("Entering tunnel!")
  ├─ Play Sound 2D (Vent_Crawl_Sound)
  ├─ Set Visibility (false)  // Hide while in tunnel
  └─ Spawn Emitter at Location (Dust_Particle)
```

**Event OnTunnelExited:**
```cpp
Event On Tunnel Exited:
  ├─ Print String ("Exiting tunnel!")
  ├─ Set Visibility (true)
  ├─ Play Animation (Exit_Emergence_Anim)
  ├─ Play Sound 2D (Vent_Kick_Sound)
  └─ Spawn Emitter at Location (Dust_Burst_Particle)
```

**Event OnTunnelTraversing:**
```cpp
Event On Tunnel Traversing:
  ├─ Progress → Lerp (0.0 to 1.0)
  │    └─ Sound Volume Multiplier
  └─ Set Audio Volume (Crawl_Sound, Progress * 0.5)
```

### Level Blueprint - Tunnel Events

**Dramatic Tunnel Emergence:**

```cpp
Begin Play:
  ├─ Get All Actors of Class (TunnelVolume)
  └─ For Each Loop:
       └─ Bind Event to On Actor Exited Tunnel

On Tunnel Exit Event:
  ├─ Branch (Is Zombie?)
  │    True:
  │      ├─ Get Exit Point Index
  │      ├─ Camera Shake (at exit location)
  │      ├─ Play Sound at Location (Vent_Break)
  │      └─ Trigger Jumpscare (if player nearby)
```

### Visual Effects

**Tunnel Entrance Particle:**
```cpp
BP_TunnelEntranceVFX:
  └─ Particle System Component
       ├─ Template: Vent_Steam
       ├─ Auto Activate: true
       └─ Location: At vent grate
```

**Attach to Entry Points:**
```cpp
Event BeginPlay (in TunnelVolume):
  ├─ For Each (Entry Point)
  └─ Spawn Actor (BP_TunnelEntranceVFX)
       └─ Attach to Component (at entry location)
```

---

## 🌐 Multiplayer Replication

### Automatic Replication

The tunnel system is fully replicated:

**TunnelNavigationAbility:**
- `TunnelData` struct replicated to all clients
- Movement updates handled on server
- Visual states synced automatically

**TunnelVolume:**
- Trigger events fire on server
- Actor tracking replicated
- Entry/exit events broadcast to all clients

### Custom Replication (Advanced)

**Replicate Custom Events:**

```cpp
// In your custom zombie class

UFUNCTION(Server, Reliable)
void Server_NotifyTunnelEntry(ATunnelVolume* Tunnel, int32 EntryIndex);

UFUNCTION(NetMulticast, Reliable)
void Multicast_PlayTunnelVFX(FVector Location, bool bIsEntry);

void AMyZombie::OnTunnelEntered_Implementation(ATunnelVolume* Tunnel, int32 EntryIndex)
{
    Super::OnTunnelEntered_Implementation(Tunnel, EntryIndex);

    if (HasAuthority())
    {
        // Server
        FTunnelEntryPoint Entry = Tunnel->GetEntryPoint(EntryIndex);
        Multicast_PlayTunnelVFX(Entry.Location, true);
    }
}
```

### Testing Multiplayer

**Dedicated Server Test:**
```
Editor → Play → Net Mode: Client
  Number of Clients: 2
  Server Game Options: -log

Watch both clients:
  Client 1: Player escapes
  Client 2: Observe zombie use tunnel
  Both clients should see smooth tunnel traversal
```

---

## 🚀 Advanced Usage

### Dynamic Tunnel Creation

**Runtime Tunnel Spawning:**

```cpp
UFUNCTION(BlueprintCallable)
ATunnelVolume* CreateTunnelAtRuntime(const TArray<FVector>& PathPoints)
{
    // Spawn tunnel
    ATunnelVolume* NewTunnel = GetWorld()->SpawnActor<ATunnelVolume>();

    // Clear default spline
    NewTunnel->TunnelSpline->ClearSplinePoints();

    // Add path points
    for (int32 i = 0; i < PathPoints.Num(); i++)
    {
        NewTunnel->TunnelSpline->AddSplinePoint(PathPoints[i], ESplineCoordinateSpace::World);
    }

    // Auto-generate entries
    NewTunnel->AutoGenerateEntryPoints();
    NewTunnel->RebuildTriggerVolumes();

    return NewTunnel;
}
```

### Conditional Tunnel Access

**Lock Tunnels Until Event:**

```cpp
// In Level Blueprint or Game Mode

Event Player Breaks Vent Grate:
  ├─ Get Tunnel Volume (by tag)
  ├─ Set bZombies Can Use: true
  └─ Play Sound (Vent_Break)

// Now zombies can use this tunnel
```

**Tunnel Unlocking System:**

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite)
TArray<FName> RequiredGameplayTags;

bool ATunnelVolume::CanActorEnter_Implementation(AActor* Actor, int32 EntryIndex)
{
    // Check gameplay tags
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor);
    if (TagInterface)
    {
        for (FName Tag : RequiredGameplayTags)
        {
            if (!TagInterface->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(Tag)))
            {
                return false; // Missing required tag
            }
        }
    }

    return Super::CanActorEnter_Implementation(Actor, EntryIndex);
}
```

### One-Way Tunnels

**Emergency Escape Vents (Exit Only):**

```cpp
Entry Point Configuration:
  Entry 0:
    bIsEntrance: false
    bIsExit: true
    EntryTag: "EmergencyExit"

// Zombies can't enter from this side
// But can exit if they entered elsewhere
```

### Tunnel Ambushes

**Hide Zombies in Tunnels:**

```cpp
Event BeginPlay:
  ├─ Spawn Zombie
  ├─ Get Tunnel Volume
  ├─ Get Component (TunnelNavigationAbility)
  ├─ Enter Tunnel (manually at entry 0)
  └─ Set Traversal Progress: 0.5  // Halfway through

// When player triggers:
Event Player Enters Room:
  └─ Resume Tunnel Traversal
       └─ Zombie emerges from vent!
```

---

## 🐛 Troubleshooting

### "Zombie doesn't enter tunnel"

**Checks:**
1. ✅ TunnelNavigationAbility added to zombie?
   - Check Components panel
2. ✅ Tunnel has bZombiesCanUse = true?
   - Check TunnelVolume settings
3. ✅ Entry point has bIsEntrance = true?
   - Check EntryPoints array
4. ✅ Trigger volumes created?
   - Click "Rebuild Trigger Volumes"
5. ✅ Nav link proxy placed and configured?
   - Check LinkedTunnel is set

**Enable Debug:**
```cpp
TunnelNavigationAbility:
  bShowDebug: true

TunnelVolume:
  bShowDebug: true
  bDrawSpline: true
  bDrawEntryPoints: true
```

### "Zombie gets stuck in tunnel"

**Fixes:**
1. Check spline path smoothness
   - Edit spline, reduce sharp turns
2. Increase TunnelMovementSpeed
   - Try 300-400 for faster traversal
3. Check tunnel geometry collision
   - Ensure no blocking volumes
4. Verify exit point is valid
   - bIsExit must be true

**Emergency Fix:**
```cpp
// In zombie Blueprint
Event Tick:
  ├─ Get Tunnel Ability
  ├─ Is In Tunnel?
  │    True:
  │      └─ Get Traversal Time
  │           └─ Branch (> 10 seconds?)
  │                True: Abort Tunnel Traversal
```

### "Nav links not working"

**Fixes:**
1. Rebuild navigation
   - Build → Build Paths
2. Check Nav Mesh covers tunnel entrances
   - Press P to visualize nav mesh
3. Verify NavLinkProxy settings:
   ```
   bSmartLinkIsRelevant: true
   bUseSmartLink: true
   ```
4. Increase nav link snap radius
5. Check AI can pathfind to tunnel entrance

### "Zombie collision issues after exiting"

**Fix:**
```cpp
TunnelNavigationAbility:
  bRestoreCollisionOnExit: true
  bRestoreMeshScaleOnExit: true

// Verify original values are cached:
BeginPlay → Check OriginalCapsuleRadius/Height
```

### "Multiplayer desync"

**Fix:**
```cpp
// Ensure server authority
if (HasAuthority())
{
    TunnelAbility->EnterTunnel(...);
}

// Check TunnelData replication
GetLifetimeReplicatedProps() includes TunnelData
```

---

## ⚡ Performance Optimization

### Optimization Tips

**1. Limit Active Tunnels:**
```cpp
// Only check nearby tunnels
FindBestTunnelToTarget(..., MaxSearchRadius: 2000.0f)

// Instead of searching all tunnels in world
```

**2. Reduce Tick Frequency:**
```cpp
UTunnelNavigationAbility:
  PrimaryComponentTick.TickInterval = 0.1f  // 10 times/second instead of every frame
```

**3. Cull Distant Tunnel Visuals:**
```cpp
Event Tick (TunnelVolume):
  ├─ Get Distance to Nearest Player
  └─ Branch (> 3000 units?)
       True: Set Spline Visibility (false)
       False: Set Spline Visibility (true)
```

**4. Pool Trigger Volumes:**
```cpp
// Don't create/destroy triggers frequently
// Create once in BeginPlay, reuse
```

**5. Limit Simultaneous Tunnel Users:**
```cpp
// In TunnelVolume
UPROPERTY(EditAnywhere)
int32 MaxSimultaneousUsers = 3;

bool CanActorEnter(...)
{
    if (ActorsInTunnel.Num() >= MaxSimultaneousUsers)
        return false;
    // ...
}
```

### Performance Metrics

**Target Performance:**
- 10 active tunnels: < 0.5ms per frame
- 5 zombies traversing: < 1.0ms per frame
- 50 trigger volumes: < 0.1ms per frame

**Profiling:**
```
stat game
stat navmesh
```

---

## 📊 Complete Example Setup

### Example: Office Building with Vent Network

**Level: Office_Survival**

**Tunnels Created:**

**1. Main Air Duct**
```cpp
TunnelVolume_MainDuct:
  Entry Points:
    - Break Room Vent (entrance only)
    - Office A Ceiling (exit)
    - Office B Ceiling (exit)
    - Hallway Vent (bidirectional)

  Settings:
    TunnelSize: Medium
    SpeedMultiplier: 0.7
    PathfindingPriority: 1.5
```

**2. Emergency Escape Vent**
```cpp
TunnelVolume_EmergencyVent:
  Entry Points:
    - Server Room (entrance only)
    - Roof Access (exit only)

  Settings:
    TunnelSize: Small
    bZombiesCanUse: false  // Locked initially
    bPlayersCanUse: true
```

**3. Pipe Crawlspace**
```cpp
TunnelVolume_Pipes:
  Entry Points:
    - Basement (entrance)
    - First Floor Bathroom (exit)
    - Second Floor Bathroom (exit)

  Settings:
    TunnelSize: Small
    SpeedMultiplier: 0.5  // Tight fit
    ZombieScaleInTunnel: 0.6  // Very cramped
```

**Zombie Configuration:**

```cpp
BP_Zombie_OfficeCrawler:
  Components:
    └─ TunnelNavigationAbility
         ├─ TunnelMovementSpeed: 220
         ├─ bAutoDetectTunnels: true
         ├─ bUseTunnelsForPathfinding: true
         ├─ bPreferTunnels: true  // Aggressive tunnel usage
         └─ MaxTunnelDetourDistance: 800
```

**Result:**
- Zombies ambush from ceiling vents
- Players can escape through emergency vent
- Dramatic tunnel emergence moments
- Dynamic cat-and-mouse gameplay

---

## 🎓 Best Practices

1. **Tunnel Design:**
   - Keep tunnels relatively short (< 20m) for pacing
   - Place exits near player objectives
   - Use tunnels for surprise, not primary navigation

2. **Balancing:**
   - Limit tunnel usage frequency
   - Make entrances visible but hard to reach
   - Add audio cues for tunnel traversal

3. **Level Design:**
   - Connect high-ground positions
   - Create alternative flanking routes
   - Balance with player defensive positions

4. **Performance:**
   - Use nav links sparingly
   - Pool trigger volumes
   - Limit active tunnel count per area

5. **Gameplay:**
   - Telegraph tunnel presence (vent grates, sounds)
   - Reward player awareness
   - Use for memorable moments

---

## 📝 Summary

You now have a complete tunnel navigation system! Zombies can:
- ✅ Detect and enter tunnels automatically
- ✅ Navigate complex pipe/vent networks
- ✅ Use tunnels in AI pathfinding
- ✅ Emerge from unexpected locations
- ✅ Create tense survival moments

**Next Steps:**
1. Create your first tunnel in a test map
2. Add TunnelNavigationAbility to zombies
3. Place NavLinkProxy
4. Test and iterate on placement
5. Add visual/audio polish

**Happy Tunneling!** 🚇🧟

---

*For issues or questions, check Troubleshooting section or enable bShowDebug for detailed logging.*
