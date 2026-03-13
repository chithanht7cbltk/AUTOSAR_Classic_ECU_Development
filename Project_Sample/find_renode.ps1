Write-Host "Searching for Renode installation..."

$possiblePaths = @(
    "C:\Program Files\Renode",
    "C:\Program Files (x86)\Renode",
    "$env:ProgramFiles\Renode",
    "${env:ProgramFiles(x86)}\Renode",
    "$env:LOCALAPPDATA\Renode"
)

foreach ($path in $possiblePaths) {
    if (Test-Path $path) {
        Write-Host "Found directory: $path"
        Get-ChildItem $path -Filter "renode.exe" -Recurse -ErrorAction SilentlyContinue | ForEach-Object {
            Write-Host "  renode.exe at: $($_.FullName)"
        }
    }
}

# Also check winget installation location
Write-Host ""
Write-Host "Checking winget packages location..."
Get-ChildItem "$env:LOCALAPPDATA\Microsoft\WinGet\Packages" -Filter "*renode*" -Recurse -ErrorAction SilentlyContinue | ForEach-Object {
    Write-Host "Found: $($_.FullName)"
}
