$msiPath = "d:\AUTOSAR Classic ECU Development\Projects\Project_Sample\Project_Sample\renode.msi"
Write-Host "Checking MSI file..."
Write-Host "Exists: $(Test-Path $msiPath)"

if (Test-Path $msiPath) {
    $file = Get-Item $msiPath
    Write-Host "File size: $($file.Length) bytes"
    
    Write-Host ""
    Write-Host "Installing Renode..."
    Start-Process "msiexec.exe" -ArgumentList "/i `"$msiPath`" /quiet /norestart" -Wait
    Write-Host "Installation completed"
} else {
    Write-Host "MSI file not found, downloading..."
    $ProgressPreference = 'SilentlyContinue'
    Invoke-WebRequest -Uri "https://github.com/renode/renode/releases/download/v1.15.1/renode-1.15.1.msi" -OutFile $msiPath
    Write-Host "Download complete"
    
    Write-Host ""
    Write-Host "Installing Renode..."
    Start-Process "msiexec.exe" -ArgumentList "/i `"$msiPath`" /quiet /norestart" -Wait
    Write-Host "Installation completed"
}

Write-Host ""
Write-Host "Searching for Renode installation..."
Get-ChildItem "C:\Program Files" -Recurse -Filter "renode.exe" -ErrorAction SilentlyContinue | ForEach-Object { Write-Host $_.FullName }
Get-ChildItem "C:\Program Files (x86)" -Recurse -Filter "renode.exe" -ErrorAction SilentlyContinue | ForEach-Object { Write-Host $_.FullName }
