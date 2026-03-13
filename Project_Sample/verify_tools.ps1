# Refresh PATH from User environment
$userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
$env:Path = "$env:Path;$userPath"

# Also add known paths directly
$env:Path = "$env:Path;C:\Program Files (x86)\GnuWin32\bin;C:\Program Files\Renode\bin"

Write-Host "=== Checking all prerequisites ==="
Write-Host ""

# Check Python
Write-Host "1. Python:"
python --version
Write-Host ""

# Check ARM GCC
Write-Host "2. ARM GCC Toolchain:"
arm-none-eabi-gcc --version
Write-Host ""

# Check Make
Write-Host "3. Make:"
make --version
Write-Host ""

# Check Renode
Write-Host "4. Renode:"
renode --version
Write-Host ""

Write-Host "=== Verification Complete ==="
