Write-Host "Checking temp files:"
Write-Host "Renode MSI: $(Test-Path "$env:TEMP\renode-1.15.1.msi")"
Write-Host "Make ZIP: $(Test-Path "$env:TEMP\make-3.81-bin.zip")"

Write-Host ""
Write-Host "Searching for renode.exe..."
Get-ChildItem "C:\Program Files" -Recurse -Filter "renode.exe" -ErrorAction SilentlyContinue | Select-Object FullName

Write-Host ""
Write-Host "Searching for make.exe..."
Get-ChildItem "C:\Program Files" -Recurse -Filter "make.exe" -ErrorAction SilentlyContinue | Select-Object FullName
Get-ChildItem "C:\" -Recurse -Filter "make.exe" -ErrorAction SilentlyContinue -Depth 2 | Select-Object FullName
