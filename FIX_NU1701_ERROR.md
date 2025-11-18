# Fix NU1701 Error - Windows 10

## What is NU1701?
NU1701 is a NuGet warning that occurs when a .NET Standard/.NET Core project references a package built for .NET Framework. This is common in Unreal Engine projects.

## Solutions (Try in order)

### **Solution 1: Suppress the Warning (Recommended)**
✅ **DONE** - I've created `Directory.Build.props` to suppress NU1701 globally.

This is safe because Unreal Engine handles .NET Framework compatibility.

---

### **Solution 2: Regenerate Visual Studio Project Files**

**In Windows:**
1. Close Visual Studio (if open)
2. Right-click on `cleanzombie.uproject`
3. Select **"Generate Visual Studio project files"**
4. Wait for generation to complete
5. Open the `.sln` file in Visual Studio
6. Build the project

---

### **Solution 3: Update NuGet Package**

The VisualStudioTools plugin uses an outdated package. Update it:

**Option A - Via NuGet Package Manager (Visual Studio):**
1. Open `cleanzombie.sln` in Visual Studio
2. Right-click solution → **Manage NuGet Packages for Solution**
3. Go to **Updates** tab
4. Update `Microsoft.VisualStudioEng.MicroBuild.Core` to latest version

**Option B - Edit packages.config manually:**
Update `Plugins/VisualStudioTools/Scripts/packages.config`:
```xml
<?xml version="1.0" encoding="utf-8"?>
<packages>
  <package id="Microsoft.VisualStudioEng.MicroBuild.Core" version="1.0.0" targetFramework="native" developmentDependency="true" />
</packages>
```

---

### **Solution 4: Clean and Rebuild**

If the error persists:

1. **Delete these folders/files:**
   - `Binaries/`
   - `Intermediate/`
   - `Saved/`
   - `.vs/` (hidden folder)
   - `*.sln`
   - `*.suo`
   - `*.user`

2. **Regenerate project files** (right-click .uproject)

3. **Rebuild in Visual Studio:**
   - Open solution
   - Build → Rebuild Solution

---

### **Solution 5: Add NoWarn to Each .csproj (If generated)**

If you have `.csproj` files, add this inside `<PropertyGroup>`:
```xml
<NoWarn>$(NoWarn);NU1701</NoWarn>
```

---

## Still Having Issues?

Run this in PowerShell (from project root):
```powershell
# Clean everything
Remove-Item -Recurse -Force Binaries, Intermediate, Saved, .vs -ErrorAction SilentlyContinue
Get-ChildItem -Filter "*.sln" -Recurse | Remove-Item -Force

# Then regenerate project files via right-click on .uproject
```

---

## Why This Happens in Unreal Engine

- Unreal Build Tool generates C# project files
- Some plugins use NuGet packages
- VisualStudioTools plugin uses MicroBuild.Core (old .NET Framework package)
- Modern Visual Studio expects .NET Standard packages
- **The warning is safe to ignore/suppress for UE projects**
