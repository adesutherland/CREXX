param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^[0-9a-f]{40}$')]
    [string]$ExpectedCommit,

    [Parameter(Mandatory = $true)]
    [string]$BuildDirectory,

    [Parameter(Mandatory = $true)]
    [string]$EvidenceDirectory,

    [ValidateRange(1, 64)]
    [int]$BuildJobs = 4,

    [ValidateRange(1, 16)]
    [int]$TestJobs = 2
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Invoke-Native {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,
        [Parameter(Mandatory = $true)]
        [scriptblock]$Command
    )

    & $Command
    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code $LASTEXITCODE"
    }
}

function Resolve-Executable {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BinDirectory,
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $candidate = Join-Path $BinDirectory "$Name.exe"
    if (Test-Path -LiteralPath $candidate) {
        return $candidate
    }
    $candidate = Join-Path $BinDirectory $Name
    if (Test-Path -LiteralPath $candidate) {
        return $candidate
    }
    throw "required installed executable is missing: $Name"
}

function Assert-SmokeOutput {
    param(
        [Parameter(Mandatory = $true)]
        [string]$LogPath,
        [Parameter(Mandatory = $true)]
        [string]$BinDirectory,
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $expectedCount = 1
    if (Test-Path -LiteralPath (Join-Path $BinDirectory 'rxtvm.exe')) {
        $expectedCount = 2
    }
    $actualCount = @(Select-String -LiteralPath $LogPath `
        -Pattern '^PASS: basic concurrency example$').Count
    if ($actualCount -ne $expectedCount) {
        throw "$Name did not report PASS for every VM"
    }
}

function Invoke-ToolchainSmoke {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,
        [Parameter(Mandatory = $true)]
        [string]$BinDirectory,
        [Parameter(Mandatory = $true)]
        [string]$OutputRoot,
        [Parameter(Mandatory = $true)]
        [string]$RepositoryRoot
    )

    $smokeDirectory = Join-Path $OutputRoot $Name
    New-Item -ItemType Directory -Force -Path $smokeDirectory | Out-Null
    Copy-Item -LiteralPath (Join-Path $RepositoryRoot 'docs/books/crexx_programming_guide/examples/concurrency_basic.crexx') -Destination $smokeDirectory

    $rxc = Resolve-Executable -BinDirectory $BinDirectory -Name 'rxc'
    $rxas = Resolve-Executable -BinDirectory $BinDirectory -Name 'rxas'
    $rxlink = Resolve-Executable -BinDirectory $BinDirectory -Name 'rxlink'
    $rxbvm = Resolve-Executable -BinDirectory $BinDirectory -Name 'rxbvm'

    Push-Location $smokeDirectory
    try {
        Invoke-Native -Name "$Name rxc" -Command {
            & $rxc -i $BinDirectory -x -o concurrency_basic concurrency_basic.crexx
        }
        Invoke-Native -Name "$Name rxas" -Command {
            & $rxas -o concurrency_basic concurrency_basic
        }
        Invoke-Native -Name "$Name rxlink" -Command {
            & $rxlink -s -o concurrency_basic_linked concurrency_basic.rxbin `
                (Join-Path $BinDirectory 'library') `
                (Join-Path $BinDirectory 'classlib')
        }
        Invoke-Native -Name "$Name rxbvm" -Command {
            & $rxbvm concurrency_basic_linked.rxbin
        }

        $rxtvmCandidate = Join-Path $BinDirectory 'rxtvm.exe'
        if (Test-Path -LiteralPath $rxtvmCandidate) {
            Invoke-Native -Name "$Name rxtvm" -Command {
                & $rxtvmCandidate concurrency_basic_linked.rxbin
            }
        }
    }
    finally {
        Pop-Location
    }
}

$repositoryRoot = (git rev-parse --show-toplevel).Trim()
$headCommit = (git rev-parse HEAD).Trim()
if ($headCommit -ne $ExpectedCommit) {
    throw "HEAD $headCommit does not match expected commit $ExpectedCommit"
}
if (@(git status --porcelain --untracked-files=all).Count -ne 0) {
    throw 'qualification checkout is not clean'
}

$repositoryRoot = [IO.Path]::GetFullPath($repositoryRoot)
$buildFull = [IO.Path]::GetFullPath($BuildDirectory)
$evidenceFull = [IO.Path]::GetFullPath($EvidenceDirectory)
$artifactFull = "$evidenceFull-artifacts"
if ($buildFull -eq $evidenceFull) {
    throw 'build and evidence directories must differ'
}
if ($buildFull -eq $artifactFull) {
    throw 'build and artifact directories must differ'
}
$repositoryPrefix = $repositoryRoot.TrimEnd([IO.Path]::DirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar
if ($buildFull.StartsWith($repositoryPrefix, [StringComparison]::OrdinalIgnoreCase) -or
    $evidenceFull.StartsWith($repositoryPrefix, [StringComparison]::OrdinalIgnoreCase) -or
    $artifactFull.StartsWith($repositoryPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'build, evidence and artifact directories must be outside the repository'
}
foreach ($directory in @($buildFull, $evidenceFull, $artifactFull)) {
    if (Test-Path -LiteralPath $directory) {
        if ((Get-ChildItem -Force -LiteralPath $directory | Measure-Object).Count -ne 0) {
            throw "qualification directory is not empty: $directory"
        }
    }
    else {
        New-Item -ItemType Directory -Force -Path $directory | Out-Null
    }
}

$labelDirectory = Join-Path $evidenceFull 'labels'
$installDirectory = Join-Path $evidenceFull 'install'
$packageDirectory = Join-Path $evidenceFull 'package'
New-Item -ItemType Directory -Force -Path $labelDirectory, $installDirectory, $packageDirectory | Out-Null
$transcriptStarted = $false
Start-Transcript -LiteralPath (Join-Path $evidenceFull 'qualification-transcript.txt') | Out-Null
$transcriptStarted = $true

try {
    $env:CREXX_HTTP_TLS_LIVE_VERIFY = '1'
    if (-not $env:CREXX_HTTP_TLS_LIVE_HOST) {
        $env:CREXX_HTTP_TLS_LIVE_HOST = 'example.com'
    }
    if (-not $env:CREXX_HTTP_TLS_MISMATCH_HOST) {
        $env:CREXX_HTTP_TLS_MISMATCH_HOST = 'wrong.host.badssl.com'
    }

    @(
        "expected_commit=$ExpectedCommit"
        "head=$headCommit"
        "platform=$([Environment]::OSVersion.VersionString)"
        "architecture=$([Runtime.InteropServices.RuntimeInformation]::OSArchitecture)"
        "cmake=$((cmake --version | Select-Object -First 1))"
        "ninja=$(ninja --version)"
        "build_jobs=$BuildJobs"
        "test_jobs=$TestJobs"
        "tls_trusted_host=$env:CREXX_HTTP_TLS_LIVE_HOST"
        "tls_mismatch_host=$env:CREXX_HTTP_TLS_MISMATCH_HOST"
    ) | Set-Content -LiteralPath (Join-Path $evidenceFull 'provenance.txt')

    Invoke-Native -Name 'configure' -Command {
        cmake -S $repositoryRoot -B $buildFull -G Ninja `
            -DCMAKE_BUILD_TYPE=MinSizeRel `
            -DCREXX_FORCE_SYSTEM_OPENSSL=ON `
            -DCREXX_CONCURRENCY_CTEST_JOBS=1 2>&1 |
            Tee-Object -FilePath (Join-Path $evidenceFull 'configure.log')
    }
    Invoke-Native -Name 'build' -Command {
        cmake --build $buildFull --parallel $BuildJobs 2>&1 |
            Tee-Object -FilePath (Join-Path $evidenceFull 'build.log')
    }
    Invoke-Native -Name 'concurrency matrix' -Command {
        cmake --build $buildFull --target concurrency-qa --parallel $BuildJobs 2>&1 |
            Tee-Object -FilePath (Join-Path $evidenceFull 'concurrency-matrix.log')
    }
    Invoke-Native -Name 'live TLS verification' -Command {
        ctest --test-dir $buildFull --parallel 1 --output-on-failure -V `
            -R '^ts_http_tls_live_' 2>&1 |
            Tee-Object -FilePath (Join-Path $evidenceFull 'tls-live.log')
    }
    Invoke-Native -Name 'stress repeat' -Command {
        ctest --test-dir $buildFull --parallel 1 --output-on-failure `
            --repeat until-fail:20 -L '^concurrency-stress$' 2>&1 |
            Tee-Object -FilePath (Join-Path $evidenceFull 'stress-repeat-20.log')
    }
    Invoke-Native -Name 'full CTest' -Command {
        ctest --test-dir $buildFull --parallel $TestJobs --output-on-failure `
            --no-tests=error 2>&1 |
            Tee-Object -FilePath (Join-Path $evidenceFull 'full-ctest.log')
    }

    $umbrellaFile = Join-Path $labelDirectory 'concurrency.txt'
    ctest --test-dir $buildFull -N -L '^concurrency$' | Set-Content -LiteralPath $umbrellaFile
    $umbrellaMatch = Select-String -LiteralPath $umbrellaFile -Pattern '^Total Tests: ([0-9]+)$'
    if (-not $umbrellaMatch -or [int]$umbrellaMatch.Matches[0].Groups[1].Value -lt 1) {
        throw 'concurrency umbrella label selected no tests'
    }
    foreach ($solutionNumber in '01','02','03','04','05','06','07','08','09') {
        $label = "concurrency-sp$solutionNumber"
        $labelFile = Join-Path $labelDirectory "$label.txt"
        ctest --test-dir $buildFull -N -L "^$label`$" | Set-Content -LiteralPath $labelFile
        $totalLine = Select-String -LiteralPath $labelFile -Pattern '^Total Tests: ([0-9]+)$'
        if (-not $totalLine -or [int]$totalLine.Matches[0].Groups[1].Value -lt 1) {
            throw "$label selected no tests"
        }
    }

    $installPrefix = Join-Path $installDirectory 'prefix'
    Invoke-Native -Name 'install' -Command {
        cmake --install $buildFull --prefix $installPrefix 2>&1 |
            Tee-Object -FilePath (Join-Path $installDirectory 'install.log')
    }
    Get-ChildItem -Recurse -File -LiteralPath $installPrefix |
        ForEach-Object FullName | Sort-Object |
        Set-Content -LiteralPath (Join-Path $installDirectory 'inventory.txt')
    $installedBin = Join-Path $installPrefix 'bin'
    $installSmokeLog = Join-Path $installDirectory 'smoke.log'
    Invoke-ToolchainSmoke -Name 'install-smoke' -BinDirectory $installedBin -OutputRoot $installDirectory -RepositoryRoot $repositoryRoot 2>&1 |
        Tee-Object -FilePath $installSmokeLog
    Assert-SmokeOutput -LogPath $installSmokeLog -BinDirectory $installedBin `
        -Name 'installed toolchain smoke'

    $payloadName = 'CREXX-windows-x64'
    $payloadRoot = Join-Path $artifactFull 'payload'
    $payloadDirectory = Join-Path $payloadRoot $payloadName
    New-Item -ItemType Directory -Force -Path (Join-Path $payloadDirectory 'bin'), (Join-Path $payloadDirectory 'examples') | Out-Null
    Copy-Item -Recurse -Force -Path (Join-Path $buildFull 'bin/*') -Destination (Join-Path $payloadDirectory 'bin')
    Copy-Item -Recurse -Force -Path (Join-Path $repositoryRoot 'examples/*') -Destination (Join-Path $payloadDirectory 'examples')
    Copy-Item -Recurse -Force -Path (Join-Path $buildFull 'example-artifacts/*') -Destination (Join-Path $payloadDirectory 'examples')
    Copy-Item -Force -Path (Join-Path $repositoryRoot 'LICENSE'), (Join-Path $repositoryRoot 'README.md'), (Join-Path $repositoryRoot 'SECURITY.md'), (Join-Path $repositoryRoot 'INSTALL-RUN.md') -Destination $payloadDirectory
    Copy-Item -Force -LiteralPath (Join-Path $buildFull 'generated/VERSION') -Destination (Join-Path $payloadDirectory 'VERSION')
    Copy-Item -Force -LiteralPath (Join-Path $buildFull 'generated/BUILDINFO') -Destination (Join-Path $payloadDirectory 'BUILDINFO')

    foreach ($requiredPayload in 'library.rxbin','classlib.rxbin','rxfnsg.rxbin') {
        if (-not (Test-Path -LiteralPath (Join-Path $payloadDirectory "bin/$requiredPayload"))) {
            throw "portable package is missing $requiredPayload"
        }
    }

    $archive = Join-Path $artifactFull "$payloadName.zip"
    Compress-Archive -Path $payloadDirectory -DestinationPath $archive -Force
    $extractDirectory = Join-Path $artifactFull 'zip-extract'
    Expand-Archive -LiteralPath $archive -DestinationPath $extractDirectory -Force
    $extractedBin = Join-Path $extractDirectory "$payloadName/bin"
    $zipSmokeLog = Join-Path $packageDirectory 'zip-smoke.log'
    Invoke-ToolchainSmoke -Name 'zip-smoke' -BinDirectory $extractedBin -OutputRoot $packageDirectory -RepositoryRoot $repositoryRoot 2>&1 |
        Tee-Object -FilePath $zipSmokeLog
    Assert-SmokeOutput -LogPath $zipSmokeLog -BinDirectory $extractedBin `
        -Name 'extracted ZIP toolchain smoke'

    Get-ChildItem -Recurse -File -LiteralPath $payloadDirectory |
        ForEach-Object FullName | Sort-Object |
        Set-Content -LiteralPath (Join-Path $packageDirectory 'inventory.txt')
    Get-FileHash -Algorithm SHA256 -LiteralPath $archive |
        ForEach-Object { "$($_.Hash.ToLowerInvariant())  $([IO.Path]::GetFileName($_.Path))" } |
        Set-Content -LiteralPath (Join-Path $packageDirectory 'packages.sha256')

    Select-String -LiteralPath (Join-Path $buildFull 'CMakeCache.txt') `
        -Pattern '^(CMAKE_BUILD_TYPE|CREXX_ENABLE_TLS|CREXX_VM_HANDLER_PANEL|CREXX_VM_PROFILING):' |
        ForEach-Object Line |
        Add-Content -LiteralPath (Join-Path $evidenceFull 'provenance.txt')

    if (@(git -C $repositoryRoot status --porcelain --untracked-files=all).Count -ne 0) {
        throw 'qualification changed the checkout'
    }

    $umbrellaCount = [int]$umbrellaMatch.Matches[0].Groups[1].Value
    $tlsBackend = (Select-String -LiteralPath (Join-Path $buildFull 'CMakeCache.txt') -Pattern '^CREXX_ENABLE_TLS:STRING=(.+)$').Matches[0].Groups[1].Value
    @(
        'PASS: initial concurrency Windows qualification'
        "commit=$ExpectedCommit"
        "concurrency_tests=$umbrellaCount"
        "tls_backend=$tlsBackend"
        "portable_archive=$([IO.Path]::GetFileName($archive))"
        "artifact_directory=$artifactFull"
    ) | Set-Content -LiteralPath (Join-Path $evidenceFull 'RESULT.txt')

    Stop-Transcript | Out-Null
    $transcriptStarted = $false
    $checksumLines = Get-ChildItem -Recurse -File -LiteralPath $evidenceFull |
        Where-Object { $_.Name -ne 'SHA256SUMS' } |
        Sort-Object FullName |
        ForEach-Object {
            $hash = Get-FileHash -Algorithm SHA256 -LiteralPath $_.FullName
            $relative = [IO.Path]::GetRelativePath($evidenceFull, $_.FullName).Replace('\', '/')
            "$($hash.Hash.ToLowerInvariant())  ./$relative"
        }
    $checksumLines | Set-Content -LiteralPath (Join-Path $evidenceFull 'SHA256SUMS')

    Write-Host "PASS: Windows qualification evidence retained at $evidenceFull"
}
finally {
    if ($transcriptStarted) {
        Stop-Transcript | Out-Null
    }
}
