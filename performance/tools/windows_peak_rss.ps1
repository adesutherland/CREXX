param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]] $CommandArgs
)

if (-not $CommandArgs -or $CommandArgs.Count -lt 1) {
    [Console]::Error.WriteLine('windows_peak_rss: missing child command')
    exit 2
}

$stdoutPath = [System.IO.Path]::GetTempFileName()
$stderrPath = [System.IO.Path]::GetTempFileName()

try {
    $executable = $CommandArgs[0]
    $childArgs = if ($CommandArgs.Count -gt 1) {
        $CommandArgs[1..($CommandArgs.Count - 1)]
    } else {
        @()
    }

    try {
        $process = Start-Process -FilePath $executable `
            -ArgumentList $childArgs `
            -WindowStyle Hidden `
            -RedirectStandardOutput $stdoutPath `
            -RedirectStandardError $stderrPath `
            -PassThru
    } catch {
        [Console]::Error.WriteLine("windows_peak_rss: could not start child: $($_.Exception.Message)")
        exit 127
    }

    [long] $peakWorkingSet = 0
    while (-not $process.HasExited) {
        try {
            $process.Refresh()
            $peakWorkingSet = [Math]::Max($peakWorkingSet, $process.PeakWorkingSet64)
        } catch {
            # The process may exit between HasExited and Refresh.
        }
        Start-Sleep -Milliseconds 10
    }

    $process.WaitForExit()
    try {
        $process.Refresh()
        $peakWorkingSet = [Math]::Max($peakWorkingSet, $process.PeakWorkingSet64)
    } catch {
        # The sampled peak remains authoritative after process teardown.
    }

    $stdout = Get-Content -LiteralPath $stdoutPath -Raw -ErrorAction SilentlyContinue
    $stderr = Get-Content -LiteralPath $stderrPath -Raw -ErrorAction SilentlyContinue
    if ($stdout) {
        [Console]::Out.Write($stdout)
    }
    if ($stderr) {
        [Console]::Error.Write($stderr)
    }

    $peakKiB = [Math]::Ceiling($peakWorkingSet / 1KB)
    [Console]::Error.WriteLine("Maximum resident set size (kbytes): $peakKiB")
    exit $process.ExitCode
} finally {
    Remove-Item -LiteralPath $stdoutPath, $stderrPath -Force -ErrorAction SilentlyContinue
}
