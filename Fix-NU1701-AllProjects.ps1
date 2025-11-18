# Fix NU1701 Error for All Unreal Engine Projects
# Run this PowerShell script as Administrator

param(
    [string]$ProjectPath = "",
    [switch]$ScanAllDrives = $false
)

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  NU1701 Error Fix for Unreal Engine Projects" -ForegroundColor Cyan
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""

# Function to create Directory.Build.props
function Create-DirectoryBuildProps {
    param([string]$path)

    $propsContent = @"
<Project>
  <PropertyGroup>
    <!-- Suppress NU1701 warning for .NET Framework package compatibility -->
    <NoWarn>`$(NoWarn);NU1701</NoWarn>
  </PropertyGroup>
</Project>
"@

    $propsFile = Join-Path $path "Directory.Build.props"

    if (Test-Path $propsFile) {
        Write-Host "  ✓ Directory.Build.props already exists" -ForegroundColor Yellow
    } else {
        $propsContent | Out-File -FilePath $propsFile -Encoding UTF8
        Write-Host "  ✓ Created Directory.Build.props" -ForegroundColor Green
    }
}

# Function to fix a single project
function Fix-UnrealProject {
    param([string]$projectPath)

    $projectName = Split-Path $projectPath -Leaf
    $projectDir = Split-Path $projectPath -Parent

    Write-Host "Found: $projectName" -ForegroundColor White
    Write-Host "Path:  $projectDir" -ForegroundColor Gray

    # Create Directory.Build.props
    Create-DirectoryBuildProps -path $projectDir

    # Check for VisualStudioTools plugin
    $uprojectContent = Get-Content $projectPath -Raw | ConvertFrom-Json
    $vsTools = $uprojectContent.Plugins | Where-Object { $_.Name -eq "VisualStudioTools" }

    if ($vsTools) {
        if ($vsTools.Enabled -eq $false) {
            Write-Host "  ⚠ VisualStudioTools is disabled (should be enabled on Windows)" -ForegroundColor Yellow
            Write-Host "    Would you like to enable it? (Y/N): " -NoNewline -ForegroundColor Cyan
            $response = Read-Host
            if ($response -eq "Y" -or $response -eq "y") {
                $vsTools.Enabled = $true
                $uprojectContent | ConvertTo-Json -Depth 10 | Out-File -FilePath $projectPath -Encoding UTF8
                Write-Host "  ✓ Enabled VisualStudioTools plugin" -ForegroundColor Green
            }
        } else {
            Write-Host "  ✓ VisualStudioTools plugin is enabled" -ForegroundColor Green
        }
    }

    # Check for packages.config in plugins
    $packagesConfig = Get-ChildItem -Path $projectDir -Filter "packages.config" -Recurse -ErrorAction SilentlyContinue
    if ($packagesConfig) {
        Write-Host "  ℹ Found NuGet packages.config files:" -ForegroundColor Cyan
        foreach ($pkg in $packagesConfig) {
            Write-Host "    - $($pkg.FullName)" -ForegroundColor Gray
        }
    }

    Write-Host "  ✓ Project fixed!" -ForegroundColor Green
    Write-Host ""
}

# Main execution
if ($ProjectPath -ne "") {
    # Fix specific project
    if (Test-Path $ProjectPath) {
        Fix-UnrealProject -projectPath $ProjectPath
    } else {
        Write-Host "Error: Project path not found: $ProjectPath" -ForegroundColor Red
        exit 1
    }
} elseif ($ScanAllDrives) {
    # Scan all drives for .uproject files
    Write-Host "Scanning all drives for Unreal Engine projects..." -ForegroundColor Yellow
    Write-Host "This may take a while..." -ForegroundColor Yellow
    Write-Host ""

    $drives = Get-PSDrive -PSProvider FileSystem | Where-Object { $_.Used -ne $null }
    $foundProjects = @()

    foreach ($drive in $drives) {
        Write-Host "Scanning $($drive.Name):\ ..." -ForegroundColor Gray
        $projects = Get-ChildItem -Path "$($drive.Name):\" -Filter "*.uproject" -Recurse -ErrorAction SilentlyContinue
        $foundProjects += $projects
    }

    if ($foundProjects.Count -eq 0) {
        Write-Host "No Unreal Engine projects found." -ForegroundColor Yellow
        exit 0
    }

    Write-Host "Found $($foundProjects.Count) Unreal Engine project(s):" -ForegroundColor Cyan
    Write-Host ""

    foreach ($project in $foundProjects) {
        Fix-UnrealProject -projectPath $project.FullName
    }
} else {
    # Scan common locations
    Write-Host "Scanning common project locations..." -ForegroundColor Yellow
    Write-Host ""

    $searchPaths = @(
        "$env:USERPROFILE\Documents\Unreal Projects",
        "$env:USERPROFILE\Documents",
        "$env:USERPROFILE\Desktop",
        "C:\UnrealProjects",
        "D:\UnrealProjects"
    )

    $foundProjects = @()

    foreach ($path in $searchPaths) {
        if (Test-Path $path) {
            Write-Host "Scanning: $path" -ForegroundColor Gray
            $projects = Get-ChildItem -Path $path -Filter "*.uproject" -Recurse -ErrorAction SilentlyContinue -Depth 3
            $foundProjects += $projects
        }
    }

    if ($foundProjects.Count -eq 0) {
        Write-Host "No Unreal Engine projects found in common locations." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Usage:" -ForegroundColor Cyan
        Write-Host "  Fix specific project:    .\Fix-NU1701-AllProjects.ps1 -ProjectPath 'C:\Path\To\Project.uproject'" -ForegroundColor White
        Write-Host "  Scan all drives:         .\Fix-NU1701-AllProjects.ps1 -ScanAllDrives" -ForegroundColor White
        exit 0
    }

    Write-Host "Found $($foundProjects.Count) Unreal Engine project(s):" -ForegroundColor Cyan
    Write-Host ""

    foreach ($project in $foundProjects) {
        Fix-UnrealProject -projectPath $project.FullName
    }
}

Write-Host "==================================================" -ForegroundColor Cyan
Write-Host "  All projects have been processed!" -ForegroundColor Green
Write-Host "==================================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Next Steps:" -ForegroundColor Cyan
Write-Host "  1. Regenerate Visual Studio project files (right-click .uproject)" -ForegroundColor White
Write-Host "  2. Open the solution in Visual Studio" -ForegroundColor White
Write-Host "  3. Build the project - NU1701 warnings should be gone!" -ForegroundColor White
Write-Host ""
