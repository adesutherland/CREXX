param(
    [ValidateSet('check', 'install', 'remove')][string]$Action = 'check',
    [string]$Root
)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Read-Metadata([string]$Directory) {
    $data = Get-Content -Raw -LiteralPath (Join-Path $Directory 'installer.json') | ConvertFrom-Json
    if ($data.schema -ne 1 -or $data.platform -ne 'windows-x64' -or
        $data.toolchain -ne 'msvc' -or $data.backend -notin @('vulkan', 'cuda')) {
        throw 'Unsupported installer metadata'
    }
    return $data
}

function Safe-Path([string]$Prefix, [string]$Name) {
    if (!$Name -or $Name -match '[\\:\r\n\t*?"<>|]' -or $Name.StartsWith('/') -or
        @($Name.Split('/') | Where-Object { !$_ -or $_ -in @('.', '..') -or $_ -match '[. ]$' }).Count) {
        throw "Unsafe package path: $Name"
    }
    $path = Join-Path $Prefix $Name
    $walk = $path
    while ($walk -and $walk -ne (Split-Path -Parent $Prefix)) {
        if (Test-Path -LiteralPath $walk) {
            $item = Get-Item -Force -LiteralPath $walk
            if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
                throw "Refusing junction/symlink: $walk"
            }
        }
        $walk = Split-Path -Parent $walk
    }
    return $path
}

function Check-Files([string]$Prefix, $Files) {
    foreach ($entry in $Files.PSObject.Properties) {
        $path = Safe-Path $Prefix $entry.Name
        if ($entry.Value -notmatch '^[a-f0-9]{64}$' -or
            !(Test-Path -LiteralPath $path -PathType Leaf) -or
            (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ine $entry.Value) {
            throw "Missing, changed or incompatible file: $path"
        }
    }
}

function Find-Core {
    # Windows' registered machine-wide core is authoritative. Portable users
    # explicitly choose a directory on the installer page or with -Root.
    $key = Get-ItemProperty 'HKLM:\SOFTWARE\CREXX\CREXX' -ErrorAction SilentlyContinue
    if ($key -and $key.InstallDir) { return [string]$key.InstallDir }
    $candidates = @(@($env:CREXX_HOME, $env:REXX_HOME) | Where-Object { $_ } |
        Where-Object { Test-Path -LiteralPath (Join-Path $_ 'core-package.json') } |
        ForEach-Object { (Resolve-Path -LiteralPath $_).Path } | Select-Object -Unique)
    if (@($candidates).Count -ne 1) {
        throw 'No unique cREXX installation found. Install the matching core or select its directory.'
    }
    return $candidates[0]
}

try {
    if (!$Root) { $Root = Find-Core }
    $Root = (Resolve-Path -LiteralPath $Root).Path.TrimEnd('\')
    $metadata = Read-Metadata $PSScriptRoot
    Check-Files $Root $metadata.core_files
    Check-Files (Join-Path $PSScriptRoot 'payload') $metadata.plugin_files
    foreach ($entry in $metadata.plugin_files.PSObject.Properties) {
        $null = Safe-Path $Root $entry.Name
        if ($metadata.core_files.PSObject.Properties.Name -contains $entry.Name) {
            throw "Plugin overlaps the core: $($entry.Name)"
        }
    }
    if ($Action -ne 'check') { throw 'Installer lifecycle implementation pending backend coexistence decision.' }
    Write-Output "Verified matching cREXX installation: $Root"
} catch {
    Write-Error $_
    exit 1
}
