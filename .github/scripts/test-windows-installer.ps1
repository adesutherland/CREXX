param(
    [Parameter(Mandatory)][string]$Installer,
    [Parameter(Mandatory)][string]$Payload,
    [Parameter(Mandatory)][string]$Commit
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

# This test changes machine environment/installer registration on a disposable runner.
if ($env:GITHUB_ACTIONS -ne 'true' -or !(Test-Path $env:RUNNER_TEMP)) {
    throw 'Run this installer smoke test only on a disposable GitHub Actions runner.'
}
if (Test-Path 'HKLM:\SOFTWARE\CREXX\CREXX') { throw 'CREXX is already installed.' }
$installRoot = Join-Path $env:RUNNER_TEMP 'CREXX installer smoke'
if (Test-Path $installRoot) { throw "Install destination already exists: $installRoot" }
$bin = Join-Path $installRoot 'bin'
$uninstallKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\CREXX'
$beforePath = [Environment]::GetEnvironmentVariable('Path', 'Machine')
foreach ($name in 'CREXX_HOME', 'REXX_HOME') {
    if ([Environment]::GetEnvironmentVariable($name, 'Machine')) {
        throw "Refusing to overwrite existing machine variable: $name"
    }
}

function Install-And-Check {
    # /D must be last; NSIS consumes the rest of the command line, including spaces.
    $process = Start-Process -FilePath $Installer -ArgumentList "/S /D=$installRoot" -Wait -PassThru
    if ($process.ExitCode -ne 0) { throw "Installer exited $($process.ExitCode)" }
    foreach ($name in 'CREXX_HOME', 'REXX_HOME') {
        if ([Environment]::GetEnvironmentVariable($name, 'Machine') -ne $installRoot) {
            throw "Installer did not set $name"
        }
    }
    $machinePath = [Environment]::GetEnvironmentVariable('Path', 'Machine')
    if (@($machinePath.Split(';') | Where-Object { $_ -eq $bin }).Count -ne 1) {
        throw 'Installed bin must occur exactly once in machine PATH'
    }
    $expectedPath = if ($beforePath) { "$beforePath;$bin" } else { $bin }
    if ($machinePath -ne $expectedPath) {
        throw 'Installation changed pre-existing machine PATH entries'
    }
    if (!(Test-Path $uninstallKey)) { throw 'Uninstaller registration is missing' }
    if (!(Select-String -Path (Join-Path $installRoot 'BUILDINFO') -SimpleMatch "commit=$Commit")) {
        throw 'Installed source commit does not match the build'
    }
    foreach ($source in Get-ChildItem -Path $Payload -Recurse -File) {
        $relative = [IO.Path]::GetRelativePath($Payload, $source.FullName)
        $installed = Join-Path $installRoot $relative
        if ((Get-FileHash $source.FullName).Hash -ne (Get-FileHash $installed).Hash) {
            throw "Installed payload differs: $relative"
        }
    }
}

try {
    Install-And-Check
    Install-And-Check # Reinstall/upgrade must not duplicate the PATH entry.
    $env:CREXX_HOME = $installRoot
    $env:REXX_HOME = $installRoot
    $env:Path = "$bin;$env:Path"
    foreach ($tool in 'crexx', 'rxc', 'rxas', 'rxvm') {
        $versionFlag = if ($tool -eq 'crexx') { '--version' } else { '-v' }
        $output = & (Join-Path $bin "$tool.exe") $versionFlag 2>&1 | Out-String
        if ($LASTEXITCODE -ne 0 -or !$output.Contains($Commit.Substring(0, 12))) {
            throw "Installed $tool version check failed: $output"
        }
        Write-Host $output.Trim()
    }
    # rxlink has no version option. Its installed bytes were checked above;
    # check startup/help here, then exercise linking through the driver below.
    $output = & (Join-Path $bin 'rxlink.exe') -h 2>&1 | Out-String
    if ($LASTEXITCODE -ne 0 -or !$output.Contains('cREXX Linker')) {
        throw "Installed linker help check failed: $output"
    }
    Push-Location $env:RUNNER_TEMP
    try {
        $output = & (Join-Path $bin 'crexx.exe') (Join-Path $installRoot 'examples/hello.crexx') 2>&1 | Out-String
        if ($LASTEXITCODE -ne 0 -or !$output.Contains('hello CREXX world!')) {
            throw "Installed compile/assemble/link/run smoke failed: $output"
        }
        Write-Host $output.Trim()
    } finally { Pop-Location }
} finally {
    $uninstaller = Join-Path $installRoot 'Uninstall.exe'
    if (Test-Path $uninstaller) {
        $process = Start-Process -FilePath $uninstaller -ArgumentList '/S' -Wait -PassThru
        if ($process.ExitCode -ne 0) { throw "Uninstaller exited $($process.ExitCode)" }
        $deadline = (Get-Date).AddSeconds(60)
        while ((Test-Path $installRoot) -and (Get-Date) -lt $deadline) { Start-Sleep -Milliseconds 250 }
    }
}
if (Test-Path $installRoot) { throw 'Uninstall left the installation directory behind' }
if ((Test-Path $uninstallKey) -or (Test-Path 'HKLM:\SOFTWARE\CREXX\CREXX')) {
    throw 'Uninstall left installer registration behind'
}
foreach ($name in 'CREXX_HOME', 'REXX_HOME') {
    if ([Environment]::GetEnvironmentVariable($name, 'Machine')) { throw "Uninstall left $name behind" }
}
$afterPath = [Environment]::GetEnvironmentVariable('Path', 'Machine')
if ($afterPath -ne $beforePath) {
    throw "Uninstall did not restore the machine PATH: before=$(ConvertTo-Json -Compress $beforePath); after=$(ConvertTo-Json -Compress $afterPath)"
}
Write-Host "Installer install/reinstall, payload, toolchain and uninstall checks passed for $Commit"
