[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$JdkPrefix,
    [Parameter(Mandatory = $true)][string]$NetRexxCompilerJar,
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '..\netrexx-native-windows')
)
$ErrorActionPreference = 'Stop'
$jdk = (Resolve-Path -LiteralPath $JdkPrefix).Path
$compiler = (Resolve-Path -LiteralPath $NetRexxCompilerJar).Path
if (Test-Path -LiteralPath $OutputDirectory) {
    throw 'Choose a fresh output directory; jpackage app images are not updated in place.'
}
$root = New-Item -ItemType Directory -Path $OutputDirectory
$classes = New-Item -ItemType Directory -Path (Join-Path $root.FullName 'classes')
foreach ($name in @('LottoModel.nrx','LottoWindow.nrx','LottoTests.nrx','LottoGuiTests.nrx')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination $classes.FullName
}
Push-Location $classes.FullName
try {
    & "$jdk\bin\java.exe" -cp $compiler org.netrexx.process.NetRexxC -noverbose LottoModel.nrx LottoWindow.nrx LottoTests.nrx LottoGuiTests.nrx
    if ($LASTEXITCODE -ne 0) { throw 'NetRexx compilation failed.' }
    & "$jdk\bin\java.exe" -cp . LottoTests
    if ($LASTEXITCODE -ne 0) { throw 'NetRexx model tests failed.' }
    & "$jdk\bin\java.exe" -cp . LottoGuiTests
    if ($LASTEXITCODE -ne 0) { throw 'NetRexx Swing tests failed.' }
    $jarClasses = New-Item -ItemType Directory -Path (Join-Path $root.FullName 'jar-classes')
    Get-ChildItem -Filter '*.class' -File | Copy-Item -Destination $jarClasses.FullName
    $jarInput = New-Item -ItemType Directory -Path (Join-Path $root.FullName 'input')
    $jar = Join-Path $jarInput.FullName 'Lotto-NetRexx.jar'
    & "$jdk\bin\jar.exe" --create --file $jar --main-class LottoWindow -C $jarClasses.FullName .
    if ($LASTEXITCODE -ne 0) { throw 'Java archive creation failed.' }
    & "$jdk\bin\jpackage.exe" --type app-image --input $jarInput.FullName --main-jar Lotto-NetRexx.jar --main-class LottoWindow --name Lotto-NetRexx --dest (Join-Path $root.FullName 'image') --app-version 1.0 --vendor 'cREXX project'
    if ($LASTEXITCODE -ne 0) { throw 'NetRexx native launcher packaging failed.' }
    Write-Output "Run image\Lotto-NetRexx\Lotto-NetRexx.exe with its complete runtime folder."
}
finally { Pop-Location }
