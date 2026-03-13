Write-Host "Checking Make installation:"
if (Test-Path "C:\Program Files\make") {
    Get-ChildItem "C:\Program Files\make" -Recurse -Filter "make.exe" | Select-Object FullName
} else {
    Write-Host "Make directory not found"
}

Write-Host ""
Write-Host "Checking Renode installation:"
if (Test-Path "C:\Program Files (x86)\Renode") {
    Get-ChildItem "C:\Program Files (x86)\Renode" -Filter "renode.exe" | Select-Object FullName
} else {
    Write-Host "Renode directory not found, checking other locations..."
    Get-ChildItem "C:\Program Files\Renode" -ErrorAction SilentlyContinue | Select-Object FullName
}
