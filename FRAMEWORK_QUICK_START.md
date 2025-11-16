# Zombie Variant Framework - Quick Start Guide

## ⚡ 5-Minute Setup

### What We Have Now

✅ **Modular Architecture** - Base zombie + ability components
✅ **Climbing System** - Already integrated as first example ability
✅ **Data-Driven Config** - Data tables for easy variant creation
✅ **Example Abilities** - Climbing, Spitter templates ready
✅ **Multiplayer Ready** - Full replication built-in

### What You Need To Do

1. **Compile the C++ code** ← Start here!
2. **Create a data table** for zombie variants
3. **Update your existing zombie Blueprint**
4. **Test and enjoy!**

---

## Step 1: Compile Code (Required)

### Using Visual Studio:

```bash
1. Close Unreal Editor
2. Right-click cleanzombie.uproject → "Generate Visual Studio project files"
3. Open cleanzombie.sln
4. Build Solution (Ctrl+Shift+B)
5. Launch Unreal Editor
```

### Build Output Should Show:
```
✓ ZombieBase.cpp
✓ ZombieAbilityComponent.cpp
✓ ClimbingAbility.cpp
✓ SpitterAbility.cpp
✓ (and other files...)
```

---

## Step 2: Create Data Table (5 minutes)

### In Unreal Editor:

1. **Right-click** in Content Browser
2. **Miscellaneous** → **Data Table**
3. **Row Structure:** Search "ZombieConfigData" and select it
4. **Name:** `DT_ZombieVariants`
5. **Open** the data table

### Add These Rows:

**Row: "BasicZombie"**
```
Variant Type: Basic
Display Name: "Basic Zombie"
Max Health: 100
Movement Speed: 300
Attack Damage: 20
Attack Range: 150
Ability Classes: (empty)
Point Value: 50
```

**Row: "ClimberZombie"**
```
Variant Type: Climber
Display Name: "Wall Climber"
Max Health: 120
Movement Speed: 200
Attack Damage: 20
Attack Range: 150
Ability Classes: [ClimbingAbility] ← Click + and search for ClimbingAbility
Point Value: 75
```

**Row: "RunnerZombie"**
```
Variant Type: Runner
Display Name: "Fast Runner"
Max Health: 60
Movement Speed: 600
Attack Damage: 15
Attack Range: 150
Ability Classes: (empty)
Point Value: 40
```

**Row: "TankZombie"**
```
Variant Type: Tank
Display Name: "Tank"
Max Health: 300
Movement Speed: 150
Attack Damage: 30
Attack Range: 200
Ability Classes: (empty)
Point Value: 100
```

---

## Step 3: Update Your Zombie Blueprint

You have two options:

### Option A: Modify Existing Zombie (Recommended)

1. **Open** `Content/HGT/AI_Zombie/BaseZombie` (or your zombie BP)
2. **Reparent Blueprint:**
   - File → Reparent Blueprint
   - Choose **ZombieBase** as new parent
3. **Set Config Reference:**
   - Default Values → Zombie Config Table → `DT_ZombieVariants`
   - Config Row Name → `BasicZombie`
4. **Compile & Save**

### Option B: Create New Zombie from Scratch

1. **Right-click** in Content Browser → Blueprint Class
2. **Parent Class:** ZombieBase
3. **Name:** `BP_ModularZombie`
4. **Open** and set:
   - Zombie Config Table → `DT_ZombieVariants`
   - Config Row Name → Choose variant
5. **Add mesh/animations** from existing zombie
6. **Compile & Save**

---

## Step 4: Test Different Variants

### Method 1: Change Config Row Name

In your zombie Blueprint:
```
Default Values:
└─► Config Row Name: "ClimberZombie"  ← Try different rows!
```

### Method 2: Runtime Initialization

In your spawn logic:
```blueprint
Spawn Actor (ZombieBase)
↓
Initialize From Config
├─► Row Name: "ClimberZombie"  ← Or "RunnerZombie", "TankZombie"
```

### Method 3: Random Variant Spawning

```blueprint
Event: Spawn Zombie
├─► Array of Variants: ["BasicZombie", "ClimberZombie", "RunnerZombie", "TankZombie"]
├─► Random Integer (0 to 3)
├─► Get Array Element
├─► Spawn Actor (ZombieBase)
└─► Initialize From Config (Random Variant Name)
```

---

## Step 5: Add Abilities to Variants

### Enable Climbing on a Zombie:

**In Data Table:**
```
Row: "ClimberZombie"
└─► Ability Classes: [ClimbingAbility]
```

**Or in Blueprint:**
```blueprint
Event Begin Play
└─► Add Ability (ClimbingAbility)
```

### Configure Climbing Behavior:

1. **Select** zombie in level or Blueprint
2. **Find** Climbing Ability component
3. **Configure:**
   ```
   ☑ Auto Climbing
   ☑ Climb When Path Blocked
   ☑ Drop To Attack
   Drop Attack Distance: 300
   ```

---

## 🎮 Testing Your Framework

### Quick Test Checklist:

- [ ] Compiled C++ code successfully
- [ ] Created `DT_ZombieVariants` data table
- [ ] Added at least 2 variant rows (Basic + Climber)
- [ ] Updated zombie Blueprint with ZombieBase parent
- [ ] Assigned config table and row name
- [ ] Placed zombie in level
- [ ] PIE (Play in Editor)
- [ ] Zombie spawns with correct stats
- [ ] Climber zombie climbs walls (if using ClimbingAbility)

### Visual Confirmation:

Enable debug to see it working:
```
Zombie Blueprint:
└─► bShowDebugInfo: ☑

Climbing Ability:
└─► bShowDebug: ☑
```

You should see:
- Health bar above zombie
- Active abilities listed
- Climb direction arrows (for climbers)
- Target lines

---

## 🚀 What's Next?

### Immediate Next Steps:

1. **Create more variants** in data table
2. **Add different ability combos:**
   - Climber + Spitter = Sniper zombie
   - Runner + Low health = Suicide bomber
3. **Integrate with your spawn system**
4. **Balance stats** (health, speed, etc.)
5. **Add visual variety** (different meshes per variant)

### Future Enhancements:

1. **Create new abilities:**
   - Explosion on death
   - Poison cloud
   - Shield/armor
   - Summoning other zombies
2. **Animation states** for each ability
3. **VFX & SFX** for ability activation
4. **Boss variants** with multiple abilities
5. **Dynamic difficulty** (more advanced variants in later waves)

---

## 🐛 Troubleshooting

### "ZombieConfigData not found"

**Fix:** Make sure you compiled the C++ code. The struct is defined in `ZombieBase.h`.

### "Zombie has no abilities even though I added them"

**Fixes:**
1. Check `InitializeFromConfig` is called (Event Begin Play)
2. Verify ability class is actually in data table row
3. Check ability component compiles without errors
4. Enable debug: `bShowDebugInfo = true`

### "Climber won't climb"

**Fixes:**
1. Make sure zombie has `ZombieClimbingMovementComponent` (not just `ClimbingAbility`)
2. Enable climbing: Climbing Ability → `bClimbingEnabled = true`
3. Set a target: Climbing Ability → Call `SetTarget(Player)`
4. Check surface is climbable (steep enough wall)

### "Changes to data table not applying"

**Fix:**
1. Save data table
2. In zombie BP, click "Refresh" or re-select config row
3. Or call `InitializeFromConfig` again at runtime

---

## 📊 Framework vs. Old System

### Before (Old Way):
```
❌ Need separate Blueprint for each zombie type
❌ Duplicate code across zombie types
❌ Hard to balance and maintain
❌ Adding new abilities = copy/paste nightmare
❌ Multiplayer replication per zombie class
```

### After (New Framework):
```
✅ One base zombie class
✅ Mix and match abilities like Lego blocks
✅ Change stats in data table (no code!)
✅ New abilities = new component class
✅ Replication handled automatically
✅ Easy to test and iterate
```

---

## 💡 Pro Tips

### Tip 1: Version Control Your Data Table

The data table is a single asset that defines all variants - commit it to git!

### Tip 2: Use Descriptive Row Names

Instead of: `Zombie1`, `Zombie2`
Use: `ClimberTank`, `FastSpitter`, `BossHybrid`

### Tip 3: Start with Stats, Add Abilities Later

1. Get basic variants working (just stat differences)
2. Then add one ability at a time
3. Test thoroughly between additions

### Tip 4: Create Ability Presets

Common combos:
- `FlankCombo`: Climbing + Speed boost
- `SniperCombo`: Climbing + Spitter
- `TankCombo`: High health + Slow + AOE damage
- `SuicideCombo`: Low health + Fast + Explode on death

### Tip 5: Use Blueprint Child Classes for Visuals

Create BP child classes of ZombieBase for different meshes/animations, but still use the same ability system.

---

## 📚 Documentation

**Detailed Guides:**
- [ZOMBIE_FRAMEWORK_GUIDE.md](ZOMBIE_FRAMEWORK_GUIDE.md) - Complete framework documentation
- [CLIMBING_SYSTEM_GUIDE.md](CLIMBING_SYSTEM_GUIDE.md) - Climbing ability details
- [ANIMATION_SETUP_GUIDE.md](ANIMATION_SETUP_GUIDE.md) - Animation setup

**Key Files:**
- `Source/cleanzombie/Public/ZombieBase.h` - Base zombie class
- `Source/cleanzombie/Public/ZombieAbilityComponent.h` - Ability base class
- `Source/cleanzombie/Public/ClimbingAbility.h` - Example ability
- `Source/cleanzombie/Public/SpitterAbility.h` - Example ranged ability

---

## ✅ Success Criteria

You'll know the framework is working when:

✅ You can spawn different zombie variants from one base class
✅ Changing data table values updates zombie stats
✅ Abilities activate and work correctly
✅ Climbers climb walls and ceilings
✅ Debug visualization shows ability states
✅ You can create a new zombie variant in < 5 minutes
✅ Adding a new ability doesn't require changing existing code

---

**Framework Status:** ✅ Ready to Use!

**What's Implemented:**
- ✅ Base zombie class with ability system
- ✅ Modular ability component architecture
- ✅ Data table configuration system
- ✅ Climbing ability (fully functional)
- ✅ Spitter ability (template ready)
- ✅ Example variant configurations
- ✅ Multiplayer replication
- ✅ Debug visualization tools

**What You Can Do Now:**
1. Create infinite zombie variants
2. Mix and match abilities
3. Balance with data tables
4. Add new abilities easily
5. Maintain clean, organized code

---

**Ready to build an army of modular zombies!** 🧟‍♂️🎮

Start with Step 1 (compile code) and you'll have working zombie variants in 15 minutes!
