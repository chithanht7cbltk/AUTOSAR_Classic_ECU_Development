# Update PATH for Make
$makePath = 'C:\Program Files (x86)\GnuWin32\bin'
$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
if ($userPath -notlike "*$makePath*") {
    [Environment]::SetEnvironmentVariable('Path', "$userPath;$makePath", 'User')
}
Write-Host "Make path added: $makePath"

# Find Renode path
$renodePath = ""
if (Test-Path "C:\Program Files\Renode\bin") { 
    $renodePath = "C:\Program Files\Renode\bin" 
} elseif (Test-Path "C:\Program Files (x86)\Renode\bin") { 
    $renodePath = "C:\Program Files (x86)\Renode\bin" 
}

if ($renodePath -ne "") {
    if ($userPath -notlike "*$renodePath*") {
        [Environment]::SetEnvironmentVariable('Path', "$userPath;$renodePath", 'User')
    }
    Write-Host "Renode path added: $renodePath"
} else {
    Write-Host "Renode not found yet (may need restart)"
}
