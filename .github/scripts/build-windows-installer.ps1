param(
    [Parameter(Mandatory)][string]$Payload,
    [Parameter(Mandatory)][string]$Installer,
    [Parameter(Mandatory)][string]$Commit
)
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$makensis = Join-Path ${env:ProgramFiles(x86)} 'NSIS\makensis.exe'
if (!(Test-Path $makensis)) {
    choco install nsis --yes --no-progress
    if ($LASTEXITCODE -ne 0) { throw 'NSIS installation failed' }
}
$Payload = (Resolve-Path -LiteralPath $Payload).Path
$Installer = [IO.Path]::GetFullPath($Installer)
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $Installer) | Out-Null
$displayVersion = (Get-Content (Join-Path $Payload 'VERSION') -First 1).Trim() -replace '^crexx-', ''
$baseVersion = $displayVersion -replace '[-+].*$', ''
$wizardBitmap = Join-Path $repoRoot 'packaging\windows\assets\crexx-wizard.bmp'
$nsisScript = Join-Path $repoRoot 'packaging\windows\crexx.nsi'
if (!(Test-Path $wizardBitmap)) { throw "Missing wizard bitmap: $wizardBitmap" }
$arguments = @(
    "/DCREXX_PAYLOAD_DIR=$Payload",
    "/DCREXX_OUTFILE=$Installer",
    "/DCREXX_DISPLAY_VERSION=$displayVersion",
    "/DCREXX_FILE_VERSION=$baseVersion.0",
    "/DCREXX_WIZARD_BITMAP=$wizardBitmap",
    $nsisScript
)
& $makensis @arguments
if ($LASTEXITCODE -ne 0) { throw 'Unsigned NSIS packaging failed' }
if ((Get-AuthenticodeSignature $Installer).Status -ne 'NotSigned') {
    throw 'Expected an unsigned snapshot installer'
}
& (Join-Path $PSScriptRoot 'test-windows-installer.ps1') -Installer $Installer -Payload $Payload -Commit $Commit
