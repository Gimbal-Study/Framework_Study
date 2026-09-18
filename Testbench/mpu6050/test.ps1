$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$output = Join-Path $PSScriptRoot 'build'
New-Item -ItemType Directory -Force $output | Out-Null
$source = Join-Path $root 'Devie/Sensor/MPU6050.c'
$includes = @(('-I' + (Join-Path $root 'Devie/Sensor')), ('-I' + (Join-Path $root 'MCAL')))
$flags = @('-std=c99', '-Wall', '-Wextra', '-Werror', '-pedantic')
$native = (Get-Command gcc.exe -ErrorAction Stop).Source
$arm = (Get-Command arm-none-eabi-gcc.exe -ErrorAction Stop).Source
$savedPath = $env:PATH
try {
$env:PATH = (Split-Path $native) + ';' + $env:PATH
& $native @flags @includes $source (Join-Path $PSScriptRoot 'test_mpu6050.c') -o (Join-Path $output 'test_mpu6050.exe')
if ($LASTEXITCODE -ne 0) { throw 'Host test compilation failed' }
& (Join-Path $output 'test_mpu6050.exe')
if ($LASTEXITCODE -ne 0) { throw 'Host tests failed' }
& $arm @flags @includes -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -c $source -o (Join-Path $output 'MPU6050.o')
if ($LASTEXITCODE -ne 0) { throw 'Cortex-M4 compilation failed' }
Write-Output 'PASS: Cortex-M4 compilation with actual MCAL header. Board/MCAL implementation not linked or flashed.'
} finally {
    $env:PATH = $savedPath
}
