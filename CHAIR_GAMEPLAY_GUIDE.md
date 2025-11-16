# 🪑 10 AWESOME CHAIR USES - Gears of War Style Zombie Shooter

**Works with your existing Blueprint SitInChairComponent!**

---

## Quick Integration

Your existing **BP_SitInChairComponent** + New **ChairActor** = Tactical Gameplay!

**Setup (30 seconds):**
1. Place `ChairActor` in level
2. Set `Chair Type` in Details
3. Configure gameplay modifiers
4. Your existing sit component handles the rest!

---

## 1. 💪 MOUNTED GUN TURRET (The Classic)

**Gears of War style - lock and load!**

### Chair Setup:
```
Chair Type: MountedGun
Damage Multiplier: 3.0
Accuracy Multiplier: 1.5
Health Regen Per Second: 0
```

### Blueprint (BP_MountedGunChair extends ChairActor):
```
Event On Actor Sat:
  ├─ Spawn Actor (BP_MountedHeavyGun)
  │    └─ Attach to Socket "GunMount"
  ├─ Give Weapon to Player
  │    ├─ Ammo: 500 rounds
  │    ├─ Fire Rate: 600 RPM
  │    └─ Damage: Base * 3.0
  ├─ Lock Camera Rotation
  │    ├─ Max Yaw: ±90 degrees
  │    └─ Max Pitch: ±45 degrees
  ├─ Spawn Muzzle Flash
  └─ Play Sound (Gun_Mount_Lock)

Event On Actor Sitting (Tick):
  ├─ If Player Firing:
  │    ├─ Camera Shake
  │    ├─ Spawn Shell Casings
  │    └─ Heat Effect
  └─ Clamp Camera to Rotation Limits

Event On Actor Unsit:
  ├─ Remove Weapon
  ├─ Destroy Gun Actor
  └─ Play Sound (Dismount)
```

### Level Design:
```
Hallway Defense:
  [Barricade] ←─── Zombies approaching
       ↑
  [Mounted Gun] ←─ Player sits here
       ↑
  [Ammo Crate]
       ↑
  [Safe Room]
```

### Gameplay:
- Lock down chokepoints
- Shred zombie hordes
- Limited rotation creates tension
- Must reposition if flanked
- Overheat mechanic (Blueprint)

**Pro Tip:** Place 2 mounted guns covering different angles for co-op play!

---

## 2. 🏥 HEALING STATION (Med Bay)

**Regenerate health between waves**

### Chair Setup:
```
Chair Type: HealingStation
Health Regen Per Second: 15.0
Damage Multiplier: 0.7  // Can't fight well while healing
Invulnerable While Sitting: false
```

### Blueprint:
```
Variables:
  - Charges: Integer = 3
  - Recharge Time: Float = 60.0

Event On Actor Sat:
  ├─ Check Charges > 0
  │    False: Play Error, Eject Player
  │    True: Consume Charge
  ├─ Spawn Particle (Medical_Mist)
  ├─ Play Sound Loop (Med_Station_Hum)
  ├─ Screen Effect (Healing Vignette)
  └─ Show Widget (Healing Progress)

Event On Actor Sitting:
  ├─ Heal Player (+15 HP/sec)
  ├─ Spawn Heal Ticks (particles)
  ├─ Update UI
  └─ If Full Health: Auto Eject

Event On Actor Unsit:
  ├─ Stop Particles
  ├─ Stop Sound
  ├─ Start Recharge Timer
  └─ Remove Widget

Timer Complete:
  ├─ Restore 1 Charge (max 3)
  ├─ Visual: Station Glows Green
  └─ Audio: Ready Beep
```

### Usage:
```
Wave Complete → Low Health (30%)
Sprint to Healing Station
Sit → Healing... 50%... 75%...
Zombie Appears!
Choice: Stay (risk) or Leave at 75%?
```

---

## 3. 🎯 SNIPER NEST (Overwatch)

**Enhanced accuracy from elevated positions**

### Chair Setup:
```
Chair Type: SniperNest
Accuracy Multiplier: 2.5
Damage Multiplier: 1.5
Vision Range Multiplier: 2.0
```

### Blueprint:
```
Event On Actor Sat:
  ├─ Set FOV (90 → 45) // Zoomed
  ├─ Enable Scope Overlay Widget
  ├─ Slow Time Dilation (Player: 0.7x)
  ├─ Highlight Weak Points
  │    ├─ Zombie Heads: Red Glow
  │    └─ Critical Spots: Yellow
  ├─ Enhanced Audio (hear farther)
  └─ Sniper Rifle Only Mode

Event On Actor Sitting:
  ├─ Stabilize Aim (reduce sway)
  ├─ Show Wind Indicator
  ├─ Distance Markers
  └─ Bullet Drop Compensation

Event On Actor Unsit:
  ├─ Reset FOV
  ├─ Remove Scope
  ├─ Normal Time
  └─ Re-enable All Weapons
```

### Map Example:
```
    [Sniper Nest] ← Player 2 (covering)
          ↓ 300m sight line
    [Street Battle] ← Player 1 (fighting)
          ↑ zombies from
    [Subway Entrance]
```

---

## 4. 🛡️ PANIC BUTTON (Safe Zone)

**Invulnerable while sitting - but can't attack!**

### Chair Setup:
```
Chair Type: SafeZone
Invulnerable While Sitting: true
Damage Multiplier: 0.0  // Cannot attack
Health Regen Per Second: 10.0
```

### Blueprint:
```
Event On Actor Sat:
  ├─ Set God Mode (true)
  ├─ Disable All Weapons
  ├─ Spawn Shield Bubble
  │    ├─ Visual: Energy Shield
  │    └─ Zombies bounce off
  ├─ Play Sound (Shield_Activate)
  └─ Show Warning: "DEFENSELESS"

Event On Actor Sitting:
  ├─ Heal Player
  ├─ Shield Pulse Effect
  └─ Count Zombies Nearby
       └─ Widget: "X zombies waiting"

Event On Actor Unsit:
  ├─ Delay 2 seconds  // Vulnerability window
  ├─ Set God Mode (false)
  ├─ Enable Weapons
  ├─ Destroy Shield
  └─ Sound: Shield_Down
```

### Tactical Use:
```
Overwhelmed by horde (10 zombies)
Sprint to Safe Chair
Sit → Shield Up → Heal
Zombies surround you (15 now!)
Call teammate: "Need backup!"
Teammate clears zombies
Stand up → Fight together
```

**Balance:** Only 1-2 safe chairs per level!

---

## 5. 🎛️ TRAP MASTER (Environmental Control)

**Activate deadly traps from control chair**

### Chair Setup:
```
Chair Type: TrapControl
Damage Multiplier: 0.5
Accuracy Multiplier: 0.8
```

### Blueprint:
```
Event On Actor Sat:
  ├─ Open Widget (Trap Control Panel)
  │    ├─ Camera Feeds (show trap zones)
  │    ├─ Button: Ceiling Spikes
  │    ├─ Button: Blast Doors
  │    ├─ Button: Electrify Floor
  │    ├─ Button: Gas Release
  │    └─ Button: Incinerator
  ├─ Highlight Trap Coverage
  └─ Enable Trap Interface

On Trap Button Pressed:
  ├─ Check Cooldown
  ├─ Get Trap Actor by ID
  ├─ Activate Trap
  │    └─ Damage all zombies in zone
  ├─ Spawn VFX (sparks, flames, etc)
  ├─ Camera Shake
  ├─ Start Cooldown Timer
  └─ Kill Feed: "20 zombies eliminated"

Event On Actor Unsit:
  └─ Close Control Panel
```

### Level Layout:
```
[Control Room] ← Player sits here
    ↓ monitors
[Room A] [Room B] [Hallway]
 Trap1     Trap2     Trap3

Zombies → Room A → Activate Trap 1
Survivors → Room B → Activate Trap 2
```

---

## 6. 🔥 LAST STAND (Die Like a Hero)

**Go out guns blazing - Gears of War execution style!**

### Chair Setup:
```
Chair Type: LastStandSeat
Damage Multiplier: 3.0
Accuracy Multiplier: 2.0
Invulnerable While Sitting: false
```

### Blueprint:
```
Variables:
  - bCommitted: Boolean = false

Event On Actor Sat:
  ├─ Show Confirmation Widget
  │    "THIS IS YOUR LAST STAND"
  │    "You cannot leave this chair"
  │    [CONFIRM] [CANCEL]
  └─ Wait for choice

On Confirm Button:
  ├─ Lock Player to Chair
  ├─ bCommitted = true
  ├─ Apply Buffs:
  │    ├─ Damage: +200%
  │    ├─ Accuracy: +100%
  │    ├─ Fire Rate: +50%
  │    └─ Infinite Ammo
  ├─ Slow Motion (0.8x)
  ├─ Epic Music
  ├─ Cinematic Letterbox
  └─ Notify Team: "X is making a stand!"

Event On Player Death (while sitting):
  ├─ Cinematic Death Cam
  ├─ Explosion (500 damage, 500 radius)
  ├─ Grant Team Buff:
  │    ├─ +30% Damage for 30 seconds
  │    └─ "Inspired by sacrifice"
  ├─ Achievement: "Hold The Line"
  ├─ Bonus Points: 1000
  └─ Highlight Reel Capture
```

### Scenario:
```
Low health, downed, no hope
Crawl to Last Stand Chair
Sit → Confirm
"Tell them I held the line..."
Kill 47 zombies before dying
Team gets damage buff
They finish mission
Your sacrifice saved them
```

---

## 7. 📻 COMMAND POST (Team Coordination)

**Call in reinforcements and coordinate team**

### Chair Setup:
```
Chair Type: CommandPost
Damage Multiplier: 0.0  // Defenseless
```

### Blueprint:
```
Event On Actor Sat:
  ├─ Open Tactical Map Widget
  │    ├─ Show Team Positions (real-time)
  │    ├─ Show Zombie Hordes
  │    ├─ Show Objectives
  │    └─ Mark Waypoints
  ├─ Enable Radio Abilities:
  │    ├─ Call Supply Drop (30s)
  │    ├─ Call Air Strike (60s)
  │    ├─ Request Evac (mission end)
  │    └─ Mark Priority Targets
  ├─ Voice: "Command, this is Alpha"
  └─ Disable Combat

On Supply Drop Requested:
  ├─ Player Must Stay Seated 30s
  ├─ Show Countdown Widget
  ├─ Zombies Attack During Call
  ├─ Team Defends Radio Operator
  ├─ Timer: 0 → Spawn Supply Crate
  └─ Cooldown: 3 minutes

On Air Strike Requested:
  ├─ Select Location on Map
  ├─ 60 second ETA
  ├─ Warning Markers
  ├─ Massive Explosion
  └─ Clear Marked Area

Event On Actor Unsit:
  └─ Close Tactical Map
```

### Co-op Example:
```
4-Player Team:
  Player 1 (Command Post): Marks targets
  Player 2-3-4: Execute orders

Player 1: "Horde incoming north!"
Team: Repositions
Player 1: "Air strike in 60 seconds"
Player 1: Marks strike zone
Team: Lures zombies to zone
BOOM - 50 zombies eliminated
```

---

## 8. 🔧 BARRICADE REPAIR (Engineer Station)

**Fix defenses while under siege**

### Chair Setup:
```
Chair Type: RepairStation
Damage Multiplier: 0.3
```

### Blueprint:
```
Event On Actor Sat:
  ├─ Open Repair Widget
  │    ├─ Show Nearby Barricades
  │    ├─ Health Bars
  │    └─ Progress Indicators
  ├─ Disable Weapons
  └─ Enable Repair Mode

Event On Actor Sitting:
  ├─ Find Barricades in Range (800 units)
  ├─ For Each Barricade:
  │    ├─ Heal (+15 HP/second)
  │    ├─ Spawn Sparks (welding effect)
  │    ├─ Play Sound (Repair_Tool)
  │    └─ Update Progress Bar
  └─ If All Full: Award Bonus Points

Event On Actor Unsit:
  └─ Stop All Repairs
```

### Gameplay:
```
Zombie wave damages 3 barricades:
  Barricade A: 40% HP
  Barricade B: 60% HP
  Barricade C: 20% HP

Player 1: Sits at Repair Station
  → Repairs all 3 simultaneously
Player 2-3: Shoot zombies over barricades
30 seconds → All barricades full
Stand up → Resume fighting
```

---

## 9. 👁️ OVERWATCH (See Everything)

**Enhanced vision and target marking**

### Chair Setup:
```
Chair Type: OverwatchStation
Vision Range Multiplier: 3.0
Accuracy Multiplier: 1.3
```

### Blueprint:
```
Event On Actor Sat:
  ├─ Enable Post Process Effect:
  │    ├─ Highlight Zombies (red outline)
  │    ├─ See Through Walls
  │    ├─ Distance Markers
  │    ├─ Threat Level (color coded)
  │    └─ Movement Trails
  ├─ Add Mark Target Ability
  ├─ Enhanced Audio (3x range)
  ├─ Zoom FOV (75)
  └─ Tactical Overlay

Event On Actor Sitting:
  ├─ Update Enemy Positions
  ├─ Track Horde Movement
  ├─ Auto-Ping Flankers
  └─ Call Outs to Team
       "20 zombies, tunnel entrance"

On Mark Target:
  ├─ Player Aims, Clicks
  ├─ Mark Zombie/Location
  ├─ Team Sees Mark (3D icon)
  ├─ Mark Lasts 10 seconds
  └─ +50% Damage to Marked Target

Event On Actor Unsit:
  ├─ Disable Vision Enhancements
  ├─ Keep Last 3 Marks Active
  └─ Fade Out Over 10s
```

### Team Play:
```
Player 1 (Overwatch): Sits on elevated chair
  "Seeing 30+ zombies, east tunnel"
Player 2: "Copy, setting mounted gun"
Player 1: "Bloater incoming, marked!"
  *Marks Bloater*
Team: Focuses fire on mark
  +50% damage → Quick kill
Player 1: "Flankers! Left side!"
Player 3: "Got them!"
```

---

## 10. 🚁 EVAC CALL (Mission Complete)

**Hold position while calling extraction**

### Chair Setup:
```
Chair Type: CommandPost
```

### Blueprint:
```
Event On Actor Sat:
  ├─ Show Radio Widget
  │    "Request Evac?"
  │    [YES] [NO]
  └─ Wait for confirmation

On YES:
  ├─ Lock Player to Chair (2 minutes)
  ├─ Voice: "We need evac NOW!"
  ├─ Response: "ETA 2 minutes, hold position"
  ├─ Start Countdown: 2:00
  ├─ Spawn Massive Horde
  ├─ Team Must Defend
  ├─ Show Countdown Widget
  └─ Cannot Leave Chair

Countdown Tick:
  ├─ 1:30 - "Halfway there!"
  ├─ 1:00 - "60 seconds!"
  ├─ 0:30 - Chopper Audio (distant)
  ├─ 0:10 - Zombies Intensify
  └─ 0:00 - EVAC ARRIVED

On Timer Complete:
  ├─ Spawn Helicopter
  ├─ Release Player from Chair
  ├─ Objective: Run to Chopper!
  ├─ Covering Fire from Chopper
  ├─ Team Boards
  ├─ Cinematic Takeoff
  └─ MISSION SUCCESS
```

### Final Mission:
```
Campaign Level 10:
  ├─ Fight to Radio Post
  ├─ Player sits, calls evac
  ├─ 2-minute holdout (epic)
  ├─ Chopper arrives
  ├─ Sprint to extraction
  ├─ Zombies chasing
  ├─ Board chopper
  └─ Credits Roll
```

---

## 🎮 Integration with Your Blueprint SitInChairComponent

Your existing component handles:
- Detecting chairs nearby
- Sitting/standing animations
- Movement disabling
- Input handling

The new ChairActor provides:
- Gameplay bonuses
- Chair types
- Events for custom logic
- Multiplayer replication

### How They Work Together:

```
Your BP_SitInChairComponent:
  Event Try Sit:
    ├─ Find Nearest Chair (ChairActor)
    ├─ Check Can Actor Sit
    ├─ Play Sit Animation
    ├─ Move to Sit Transform
    ├─ Call: Chair → Set Occupant(Self)
    └─ Chair fires OnActorSat event

ChairActor:
  Event On Actor Sat:
    └─ Your custom gameplay logic!
```

### Example Blueprint Function:

```
Function: Interact With Chair

Get Component (SitInChairComponent)
  ├─ Find Nearest Actor of Class (ChairActor)
  ├─ Branch (Chair Valid?)
  │    True:
  │      ├─ Check Chair Can Actor Sit
  │      ├─ Play Sit Animation
  │      ├─ Set Actor Transform (Chair Sit Transform)
  │      ├─ Call: Chair Set Occupant
  │      └─ Disable Movement
  │    False:
  │      └─ Print: "No chair nearby"
```

---

## 📊 Chair Type Quick Reference

| Type | Damage | Accuracy | Heal | Invuln | Best For |
|------|--------|----------|------|--------|----------|
| MountedGun | 3.0x | 1.5x | 0 | No | Chokepoints |
| HealingStation | 0.7x | 1.0x | 15/s | No | Between waves |
| SniperNest | 1.5x | 2.5x | 0 | No | Overwatch |
| SafeZone | 0x | 0x | 10/s | Yes | Panic button |
| CommandPost | 0x | 0x | 0 | No | Team leader |
| TrapControl | 0.5x | 0.8x | 0 | No | Tactical |
| RepairStation | 0.3x | 0.5x | 0 | No | Engineer |
| OverwatchStation | 1.0x | 1.3x | 0 | No | Spotter |
| LastStandSeat | 3.0x | 2.0x | 0 | No | Heroic death |

---

## 🏆 Achievement Ideas

1. **"Mounted Defense"** - 100 kills from mounted gun
2. **"Medic!"** - Heal 5000 HP at healing stations
3. **"Last Stand Hero"** - Kill 50+ zombies before dying in last stand
4. **"Trap Master"** - Kill 500 zombies with traps
5. **"Overwatch"** - Mark 1000 enemies
6. **"Hold The Line"** - Defend chair for 10 minutes
7. **"Chair Collector"** - Use all 10 chair types
8. **"Radio Operator"** - Call 50 supply drops
9. **"Engineer"** - Repair 10,000 barricade HP
10. **"Extraction"** - Complete 10 evac sequences

---

## 💡 Pro Tips

1. **Combo Placement:** Put healing station behind mounted gun
2. **Team Roles:** Assign players to specific chair types
3. **Dynamic Spawns:** More players = more zombies = more chair power
4. **Destructible Chairs:** Zombies can break them (adds pressure)
5. **Chair Tiers:** Bronze/Silver/Gold with increasing power
6. **Cooldown System:** Limit chair usage frequency
7. **Environmental Theme:** Match chairs to level (office, military, etc.)
8. **Ultimate Chair:** One super-powered chair per map (fight over it!)
9. **Chair Challenges:** Complete objectives while sitting
10. **Mobility:** Make some chairs deployable/portable

---

## 🎬 Cinematic Moments

### Campaign Sequence Example:

```
Level 5: "Defense of the Power Station"

Objective 1: Reach Control Room
  └─ Fight through zombies

Objective 2: Call for Backup
  ├─ Sit at Radio Post
  ├─ Hold 60 seconds
  └─ Reinforcements arrive (NPC marines)

Objective 3: Defend Generator
  ├─ 2 Mounted Guns available
  ├─ 1 Repair Station
  ├─ 3-minute horde
  └─ Use trap controls

Objective 4: Escape
  ├─ Sit at Evac Chair
  ├─ Call chopper (2 min)
  ├─ Final stand
  └─ Extraction

Boss Twist:
  Player in Last Stand chair
  Takes down Boss Zombie
  Team escapes thanks to sacrifice
  Post-credits: "In memory of..."
```

---

## ✨ Summary

You now have 10 tactical chair uses perfect for your Gears of War style zombie shooter:

1. ✅ **Mounted Gun** - Heavy firepower defense
2. ✅ **Healing Station** - HP regeneration
3. ✅ **Sniper Nest** - Precision overwatch
4. ✅ **Safe Zone** - Emergency invulnerability
5. ✅ **Trap Master** - Environmental kills
6. ✅ **Last Stand** - Heroic sacrifice
7. ✅ **Command Post** - Team coordination
8. ✅ **Repair Station** - Fix barricades
9. ✅ **Overwatch** - Enhanced vision
10. ✅ **Evac Call** - Mission complete

**All working with your existing Blueprint SitInChairComponent!**

Just place ChairActors, configure the type, and let your Blueprint component do the sitting mechanics!

🪑💪🧟 **Now go create epic chair combat moments!**
