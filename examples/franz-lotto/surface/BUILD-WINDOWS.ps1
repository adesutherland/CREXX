[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$CorePrefix,
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '..\native-windows'),
    [string]$GtkDataPrefix
)
$ErrorActionPreference = 'Stop'
$prefix = (Resolve-Path -LiteralPath $CorePrefix).Path
$binary = Join-Path $prefix 'bin'
foreach ($name in @('crexx.exe','rxc.exe','rxas.exe','rxlink.exe','rxcpack.exe',
        'crexx-provider-package.exe','ui_ansi.rxbin','ui_gtk.rxbin',
        'providers\rxconsole.lib','providers\rx_ui_gtk_native.lib',
        'providers\rx_ui_gtk_native.native.json')) {
    if (-not (Test-Path -LiteralPath (Join-Path $binary $name))) {
        throw "The matched development core/UI SDK is missing $name. Released Beta 3 is for the original procedural example."
    }
}
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    throw 'Run this build in a Visual Studio x64 developer PowerShell environment.'
}
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$output = (Resolve-Path -LiteralPath $OutputDirectory).Path
$modelRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..\crexx')).Path
$previousHome = $env:CREXX_HOME
try {
    $env:CREXX_HOME = $prefix
    foreach ($hostName in @('Console','GTK')) {
        $libraries = @('ui_ansi','ui_dialogs','ui_terminal_view')
        if ($hostName -eq 'GTK') {
            $libraries = @('ui_gtk')
        }
        $libraries += @('ui_local_resources','ui_contract','ui_catalog','ui')
        $stem = Join-Path $output "Lotto-$hostName"
        $arguments = @('--program',$stem,'--native','--noexec','--nocolour',
                '--jobs','1','-s',$PSScriptRoot,'-s',$modelRoot)
        foreach ($library in $libraries) { $arguments += @('-l',$library) }
        $arguments += @((Join-Path $PSScriptRoot "lotto_$($hostName.ToLower()).crexx"),
                (Join-Path $PSScriptRoot 'lotto_surface.crexx'),
                (Join-Path $modelRoot 'lotto_logic.crexx'))
        & (Join-Path $binary 'crexx.exe') @arguments
        if ($LASTEXITCODE -ne 0) { throw "Native $hostName build failed; do not use stale output." }
        if ($hostName -eq 'GTK') {
            # The packaged main already uses the CRT entry point. Give this
            # desktop launcher the Windows subsystem before signing it.
            & editbin.exe /SUBSYSTEM:WINDOWS "$stem.exe"
            if ($LASTEXITCODE -ne 0) { throw 'Cannot set the GTK desktop subsystem.' }
        }
    }
    # Keep the matching compiler runtime beside both executables.
    if ($env:VCToolsRedistDir) {
        $crt = Get-ChildItem -LiteralPath (Join-Path $env:VCToolsRedistDir 'x64') -Directory |
                Where-Object Name -Like 'Microsoft.VC*.CRT' | Select-Object -First 1
        if ($crt) { Get-ChildItem -LiteralPath $crt.FullName -Filter '*.dll' -File | Copy-Item -Destination $output }
    }
    # GTK data is separate from its DLL dependency manifest. Use the matched
    # package's retained data, or its explicit SDK prefix when building locally.
    $dataRoot = Join-Path $prefix 'bin\providers\rx_ui_gtk_native.data'
    if ($GtkDataPrefix) { $dataRoot = (Resolve-Path -LiteralPath $GtkDataPrefix).Path }
    $schemaSource = Join-Path $dataRoot 'share\glib-2.0\schemas'
    if (-not (Test-Path -LiteralPath (Join-Path $schemaSource 'gschemas.compiled'))) {
        throw 'The GTK EXE built, but its portable data is incomplete: supply -GtkDataPrefix or the matched packaged GTK data.'
    }
    foreach ($directory in @('share\glib-2.0\schemas','share\icons\Adwaita',
            'share\icons\AdwaitaLegacy','share\icons\hicolor','share\mime',
            'share\gtk-3.0','lib\gdk-pixbuf-2.0\2.10.0')) {
        $source = Join-Path $dataRoot $directory
        if (-not (Test-Path -LiteralPath $source)) { throw "Missing portable GTK data directory: $source" }
        $target = Join-Path $output $directory
        New-Item -ItemType Directory -Path $target -Force | Out-Null
        Get-ChildItem -LiteralPath $source | Copy-Item -Destination $target -Recurse -Force
    }
    $loaderCache = Get-Content -LiteralPath (Join-Path $output 'lib\gdk-pixbuf-2.0\2.10.0\loaders.cache') -Raw
    if ($loaderCache -match '"[A-Za-z]:[/\\]') {
        throw 'The GTK loader cache contains an absolute Windows path; use a relocatable SDK cache.'
    }
    Write-Output "Built both native examples in $output. Keep the GTK DLLs, share and lib folders beside the EXEs."
}
finally { $env:CREXX_HOME = $previousHome }
