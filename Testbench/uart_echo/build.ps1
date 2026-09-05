param([switch]$Batch)
$ErrorActionPreference = 'Stop'
$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$outputDir = Join-Path $PSScriptRoot $(if ($Batch) { 'build-batch' } else { 'build' })
New-Item -ItemType Directory -Force -Path $outputDir | Out-Null
$compiler = (Get-Command arm-none-eabi-gcc.exe -ErrorAction Stop).Source
$objcopy = Join-Path (Split-Path $compiler) 'arm-none-eabi-objcopy.exe'
$sources = @(
    'Core/main.c',
    'Core/System/system_stm32f4xx.c',
    'Core/System/clock.c',
    'MCAL/Target/STM32F4xx/stm32_uart.c',
    'MCAL/Target/STM32F4xx/stm32_queue.c',
    'MCAL/Target/STM32F4xx/stm32_timer.c',
    'Testbench/uart_echo/uart_echo_test.c',
    'Core/System/systick.c',
    'Board/NUCLEO_F411RE/crt0.s'
)
if ($Batch) {
    $sources = $sources | ForEach-Object {
        if ($_ -eq 'Testbench/uart_echo/uart_echo_test.c') {
            'Testbench/uart_echo/uart_batch_test.c'
        } else { $_ }
    }
}
$cpuFlags = @('-mcpu=cortex-m4', '-mthumb', '-mfpu=fpv4-sp-d16', '-mfloat-abi=hard')
$compileFlags = $cpuFlags + @('-DSTM32F411xE', '-std=gnu99', '-O3', '-Wall', '-g', '-fno-builtin', '-funsigned-char', '-fno-strict-aliasing', '-fno-common')
$includes = @('.', 'Board/NUCLEO_F411RE', 'MCAL', 'Core/System', 'CMSIS/Core', 'Test', 'CMSIS/Device/STM32F4xx')
$includeFlags = @()
foreach ($directory in $includes) { $includeFlags += '-I' + (Join-Path $projectRoot $directory) }
$objects = @()
foreach ($source in $sources) {
    $objectPath = Join-Path $outputDir (($source -replace '[/\\]', '_') + '.o')
    & $compiler @compileFlags @includeFlags -c (Join-Path $projectRoot $source) -o $objectPath
    if ($LASTEXITCODE -ne 0) { throw "Compile failed: $source" }
    $objects += $objectPath
}
$elf = Join-Path $outputDir 'uart_echo.elf'
$bin = Join-Path $outputDir 'uart_echo.bin'
$map = Join-Path $outputDir 'uart_echo.map'
$linkerScript = Join-Path $outputDir 'uart_echo.lds'
$linkerText = Get-Content -Raw (Join-Path $projectRoot 'rom_0x08000000.lds')
if (-not $linkerText.Contains('KEEP(Board/NUCLEO_F411RE/crt0.o(.text))')) {
    throw 'Unexpected startup selector in the existing linker script'
}
$linkerText.Replace('KEEP(Board/NUCLEO_F411RE/crt0.o(.text))', 'KEEP(*Board_NUCLEO_F411RE_crt0.s.o(.text))') | Set-Content -Encoding utf8 $linkerScript
Push-Location $projectRoot
try {
    & $compiler @cpuFlags @objects '--specs=nano.specs' '--specs=nosys.specs' '-u' '_printf_float' '-u' '_scanf_float' '-nostartfiles' '-ffreestanding' "-Wl,-Map=$map" '-Wl,--cref' '-Wl,-EL' '-T' $linkerScript '-lc' '-lgcc' '-o' $elf
    if ($LASTEXITCODE -ne 0) { throw 'Link failed' }
    & $objcopy -O binary $elf $bin
    if ($LASTEXITCODE -ne 0) { throw 'Binary conversion failed' }
} finally { Pop-Location }
Write-Output "Built: $elf"
Write-Output "Built: $bin"
Write-Output 'No firmware was flashed. Existing source files and build outputs were not changed.'
