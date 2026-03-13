# Check if file exists
$msiPath = "C:\Users\ADMin\renode.msi"
Write-Host "MSI file exists: $(Test-Path $msiPath)"
if (Test-Path $msiPath) {
    $file = Get-Item $msiPath
    Write-Host "File size: $($file.Length) bytes"
}

# Try installing
Write-Host ""
Write-Host "Attempting installation..."
Start-Process msiexec.exe -Wait -ArgumentList "/i `"$msiPath`" /quiet /norestart"
Write-Host "Installation command completed"

# Check if Renode installed
Write-Host ""
Write-Host "Checking for Renode..."
Get-ChildItem "C:\Program Files" -Recurse -Filter "renode.exe" -ErrorAction SilentlyContinue | Select-Object FullName
Get-ChildItem "C:\Program Files (x86)" -Recurse -Filter "renode.exe" -ErrorAction SilentlyContinue | Select-Object FullName
