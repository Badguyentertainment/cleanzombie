# Fix NU1701 Error for H:\Cplusszombiegame-main

## Quick Fix (Manual - Do This First)

### **Step 1: Create Directory.Build.props**

1. **Navigate to:** `H:\Cplusszombiegame-main\`
2. **Create a new file** named: `Directory.Build.props`
3. **Paste this content:**

```xml
<Project>
  <PropertyGroup>
    <!-- Suppress NU1701 warning for .NET Framework package compatibility -->
    <NoWarn>$(NoWarn);NU1701</NoWarn>
  </PropertyGroup>
</Project>
```

4. **Save the file** (make sure it's saved as `.props`, not `.props.txt`)

---

### **Step 2: Regenerate Visual Studio Project Files**

1. **Navigate to:** `H:\Cplusszombiegame-main\`
2. **Find** the `.uproject` file (e.g., `Cplusszombiegame.uproject`)
3. **Right-click** on the `.uproject` file
4. Select **"Generate Visual Studio project files"**
5. **Wait** for the process to complete

---

### **Step 3: Clean and Rebuild**

1. **Delete these folders** in `H:\Cplusszombiegame-main\`:
   - `Binaries\`
   - `Intermediate\`
   - `Saved\`
   - `.vs\` (hidden folder - enable "Show hidden files" in Windows Explorer)
   - Delete any `.sln` files

2. **Regenerate project files again** (right-click `.uproject`)

3. **Open the solution** in Visual Studio

4. **Build → Rebuild Solution**

5. ✅ **NU1701 error should be gone!**

---

## Automatic Fix Using the Script

You can also use the universal fix script I created:

### **Option 1: Specific Project Fix**

Open PowerShell as Administrator and run:

```powershell
cd "H:\cleanzombie"
.\Fix-NU1701-AllProjects.ps1 -ProjectPath "H:\Cplusszombiegame-main\Cplusszombiegame.uproject"
```

*(Replace `Cplusszombiegame.uproject` with the actual .uproject filename)*

---

### **Option 2: Scan H:\ Drive**

The universal script can scan the entire H:\ drive:

```powershell
cd "H:\cleanzombie"
.\Fix-NU1701-AllProjects.ps1 -ScanAllDrives
```

This will find and fix ALL Unreal projects on all drives, including H:\

---

## If VisualStudioTools Plugin is Missing

Check if `Cplusszombiegame.uproject` has the VisualStudioTools plugin:

1. **Open** `H:\Cplusszombiegame-main\Cplusszombiegame.uproject` in a text editor
2. **Look for** this section in the "Plugins" array:

```json
{
  "Name": "VisualStudioTools",
  "Enabled": true,
  "SupportedTargetPlatforms": [
    "Win64"
  ]
}
```

3. If it's **missing**, add it to the `"Plugins": [` array
4. If it's **disabled** (`"Enabled": false`), change to `"Enabled": true`
5. **Save** the file

---

## Verify the Fix

After applying the fix and rebuilding:

1. **Open** Visual Studio
2. **View → Output** window
3. **Build** the project
4. **Search** the output for "NU1701"
5. Should say: **"0 Warning(s)"** ✅

---

## Copy Fix Files to H:\ Drive

You can copy these files from the cleanzombie project to `H:\Cplusszombiegame-main\`:

**Files to copy:**
- `Directory.Build.props` → `H:\Cplusszombiegame-main\Directory.Build.props`
- `Fix-NU1701-AllProjects.ps1` → `H:\Cplusszombiegame-main\Fix-NU1701-AllProjects.ps1`
- `Fix-NU1701-AllProjects.bat` → `H:\Cplusszombiegame-main\Fix-NU1701-AllProjects.bat`

Then just double-click the `.bat` file!

---

## Troubleshooting

### Error: "Cannot find .uproject file"

Make sure you're using the correct filename:
```powershell
# List all .uproject files in the directory
Get-ChildItem "H:\Cplusszombiegame-main\" -Filter "*.uproject"
```

### Error: "Access Denied"

Run PowerShell as Administrator:
1. Press **Windows Key**
2. Type **"PowerShell"**
3. **Right-click** "Windows PowerShell"
4. Select **"Run as administrator"**

### NU1701 Still Appears

Follow the **Step 3: Clean and Rebuild** section above - delete all generated files and rebuild from scratch.

---

## Summary

**Fastest Fix:**
1. Create `Directory.Build.props` in `H:\Cplusszombiegame-main\`
2. Regenerate Visual Studio files
3. Rebuild project
4. Done! ✅

**Or use the automated script for all projects at once!**
