$makeDir = "C:\make"
$renodeInstaller = "d:\AUTOSAR Classic ECU Development\Projects\Project_Sample\Project_Sample\renode.msi"

Write-Host "=== Installing Make ==="
$makeUrl = "https://gnulinux.mirror.liquidtelecom.com/gnuwin32/make-3.81-bin.zip"
try {
    $ProgressPreference = 'SilentlyContinue'
    Invoke-WebRequest -Uri $makeUrl -OutFile "$env:TEMP\make.zip" -UseBasicParsing
    Write-Host "Make downloaded successfully"
    
    if (!(Test-Path $makeDir)) {
        New-Item -ItemType Directory -Path $makeDir -Force | Out-Null
    }
    Expand-Archive -Path "$env:TEMP\make.zip" -DestinationPath $makeDir -Force
    Write-Host "Make extracted to $makeDir"
    
    # Add to PATH
    $makeBinPath = "$makeDir\bin"
    $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
    if ($userPath -notlike "*$makeBinPath*") {
        [Environment]::SetEnvironmentVariable('Path', "$userPath;$makeBinPath", 'User')
        Write-Host "Make added to PATH"
    }
} catch {
    Write-Host "Make installation failed: $_"
}

Write-Host ""
Write-Host "=== Installing Renode ==="
$renodeUrl = "https://github.com/renode/renode/releases/download/v1.14.0/renode-1.14.0.msi"
try {
    $ProgressPreference = 'SilentlyContinue'
    Invoke-WebRequest -Uri $renodeUrl -OutFile $renodeInstaller -UseBasicParsing
    Write-Host "Renode downloaded to $renodeInstaller"
    
    Write-Host "Installing Renode..."
    Start-Process "msiexec.exe" -ArgumentList "/i `"$renodeInstaller`" /quiet /norestart" -Wait
    Write-Host "Renode installation completed"
    
    # Find and add Renode to PATH
    $renodePath = ""
    if (Test-Path "C:\Program Files\Renode") { $renodePath = "C:\Program Files\Renode" }
    elseif (Test-Path "C:\Program Files (x86)\Renode") { $renodePath = "C:\Program Files (x86)\Renode" }
    
    if ($renodePath -ne "") {
        $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
        if ($userPath -notlike "*$renodePath*") {
            [Environment]::SetEnvironmentVariable('Path', "$userPath;$renodePath", 'User')
            Write-Host "Renode added to PATH: $renodePath"
        }
    }
} catch {
    Write-Host "Renode installation failed: $_"
}

Write-Host ""
Write-Host "=== Installation Complete ==="
