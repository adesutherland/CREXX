param([Parameter(Mandatory=$true)][string]$Root)
$ErrorActionPreference = 'Stop'
$pin = Get-Content (Join-Path $PSScriptRoot '../.github/llama/vulkan-windows.json') -Raw | ConvertFrom-Json
$stamp = Join-Path $Root 'crexx-installer-sha256.txt'
if (!(Test-Path $stamp) -or (Get-Content $stamp -Raw).Trim() -ne $pin.sha256) {
    $installer = Join-Path $env:RUNNER_TEMP 'crexx-vulkan-sdk.exe'
    Invoke-WebRequest -Uri $pin.url -OutFile $installer -TimeoutSec 1800
    if ((Get-Item $installer).Length -ne $pin.bytes -or
        (Get-FileHash $installer -Algorithm SHA256).Hash.ToLowerInvariant() -ne $pin.sha256) {
        throw 'Vulkan SDK installer identity mismatch'
    }
    # LunarG's documented copy-only mode leaves registry/driver setup alone.
    $process = Start-Process -FilePath $installer -ArgumentList @(
        '--root', ('"' + $Root + '"'), '--accept-licenses', '--default-answer',
        '--confirm-command', 'install', 'copy_only=1') -Wait -PassThru
    if ($process.ExitCode -ne 0) { throw "Vulkan SDK installer failed: $($process.ExitCode)" }
    Set-Content -Path $stamp -Value $pin.sha256
    Remove-Item $installer
}
foreach ($relative in @('Include/vulkan/vulkan.h', 'Lib/vulkan-1.lib', 'Bin/glslc.exe')) {
    if (!(Test-Path (Join-Path $Root $relative))) { throw "Incomplete Vulkan SDK: $relative" }
}
"VULKAN_SDK=$Root" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
(Join-Path $Root 'Bin') | Out-File -FilePath $env:GITHUB_PATH -Encoding utf8 -Append
& (Join-Path $Root 'Bin/glslc.exe') --version
if ($LASTEXITCODE -ne 0) { throw 'Vulkan shader compiler did not start' }
