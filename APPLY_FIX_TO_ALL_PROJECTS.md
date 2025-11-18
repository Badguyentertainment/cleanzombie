# Apply NU1701 Fix to All Your Unreal Engine Projects

## Quick Start (Windows 10)

### **Method 1: Automatic Scan (Easiest)**

1. **Right-click** on `Fix-NU1701-AllProjects.bat`
2. Select **"Run as Administrator"**
3. The script will automatically:
   - Scan common project locations
   - Find all `.uproject` files
   - Apply the NU1701 fix to each project
   - Show you the results

---

### **Method 2: Scan All Drives (Thorough)**

If Method 1 doesn't find your projects:

1. **Right-click** on `Fix-NU1701-AllProjects.ps1`
2. Select **"Run with PowerShell"**
3. Or run this command in PowerShell (as Administrator):
   ```powershell
   .\Fix-NU1701-AllProjects.ps1 -ScanAllDrives
   ```

This will scan **ALL drives** (C:\, D:\, etc.) for Unreal projects.

---

### **Method 3: Fix Specific Project**

If you know the exact project path:

**PowerShell:**
```powershell
.\Fix-NU1701-AllProjects.ps1 -ProjectPath "C:\MyProjects\MyGame\MyGame.uproject"
```

**Or drag & drop:**
1. Open PowerShell as Administrator
2. Type: `.\Fix-NU1701-AllProjects.ps1 -ProjectPath "`
3. Drag your `.uproject` file into the window
4. Type: `"` and press Enter

---

## What the Script Does

For each Unreal Engine project found:

✅ Creates `Directory.Build.props` to suppress NU1701 warning
✅ Checks if VisualStudioTools plugin is enabled (asks to enable if disabled)
✅ Lists any NuGet package dependencies
✅ Shows you a summary of changes

---

## After Running the Script

**For EACH project:**

1. **Navigate to the project folder**

2. **Regenerate Visual Studio project files:**
   - Right-click on `ProjectName.uproject`
   - Select **"Generate Visual Studio project files"**
   - Wait for completion

3. **Open in Visual Studio:**
   - Double-click the `.sln` file
   - Build → Rebuild Solution

4. **Verify the fix:**
   - Check the Output window
   - NU1701 warnings should be gone!

---

## Troubleshooting

### "Script cannot be loaded" Error

If you get an execution policy error:

**PowerShell (as Administrator):**
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

Then run the script again.

---

### Script Doesn't Find Your Projects

**Option 1:** Use the full drive scan:
```powershell
.\Fix-NU1701-AllProjects.ps1 -ScanAllDrives
```

**Option 2:** Specify the exact path:
```powershell
.\Fix-NU1701-AllProjects.ps1 -ProjectPath "D:\Dev\MyProject\MyProject.uproject"
```

---

### NU1701 Still Appears After Fix

1. **Close Visual Studio completely**

2. **Delete these folders in your project:**
   - `Binaries\`
   - `Intermediate\`
   - `Saved\`
   - `.vs\` (hidden folder - enable "Show hidden files")
   - Delete all `.sln` files

3. **Regenerate project files** (right-click .uproject)

4. **Open and rebuild** in Visual Studio

5. If still seeing errors, check `FIX_NU1701_ERROR.md` for advanced solutions

---

## Manual Fix (If Scripts Don't Work)

For each project, manually create `Directory.Build.props` in the project root:

```xml
<Project>
  <PropertyGroup>
    <NoWarn>$(NoWarn);NU1701</NoWarn>
  </PropertyGroup>
</Project>
```

Save it next to your `.uproject` file, then regenerate Visual Studio files.

---

## Files Created in Each Project

- `Directory.Build.props` - Suppresses NU1701 warning globally
- (No other files are modified)

---

## Why This Works

The NU1701 warning occurs because:
- Unreal Engine's VisualStudioTools plugin uses `Microsoft.VisualStudioEng.MicroBuild.Core`
- This package is built for .NET Framework
- Modern Visual Studio expects .NET Standard packages
- The warning is **safe to suppress** for Unreal Engine projects

`Directory.Build.props` tells MSBuild to ignore this warning for all projects in the directory.

---

## Need More Help?

See `FIX_NU1701_ERROR.md` for:
- Detailed explanations
- 5 different solution methods
- Advanced troubleshooting
- Package update instructions
