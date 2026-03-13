$makePath = 'C:\Program Files\make\bin'
$renodePath = 'C:\Program Files (x86)\Renode'

# Add to User PATH
$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
$newPath = $userPath

if ($userPath -notlike "*$makePath*") {
    $newPath = "$newPath;$makePath"
}
if ($userPath -notlike "*$renodePath*") {
    $newPath = "$newPath;$renodePath"
}

[Environment]::SetEnvironmentVariable('Path', $newPath, 'User')
Write-Host "PATH updated successfully!"
Write-Host "Make path: $makePath"
Write-Host "Renode path: $renodePath"
