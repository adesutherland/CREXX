param(
    [Parameter(Mandatory = $true)]
    [string]$Java,

    [Parameter(Mandatory = $true)]
    [string]$ClassDirectory,

    [Parameter(Mandatory = $true)]
    [string]$RuntimeJar,

    [Parameter(Mandatory = $true)]
    [string]$MainClass
)

Set-StrictMode -Version Latest

$classPath = [string]::Join(
    [IO.Path]::PathSeparator,
    @($ClassDirectory, $RuntimeJar)
)
& $Java -cp $classPath $MainClass
exit $LASTEXITCODE
