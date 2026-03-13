# Refresh PATH
$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
$env:Path = "$env:Path;$userPath"
$env:Path = "$env:Path;C:\Program Files (x86)\GnuWin32\bin"

# Run make command
Set-Location "d:\AUTOSAR Classic ECU Development\Projects\Project_Sample\Project_Sample"

Write-Host "=== Cleaning build directory ==="
make clean-all

Write-Host ""
Write-Host "=== Building with Renode target ==="
make TARGET_ENV=renode
