# Windows qualification commands

Commands ran from the clean repository root on Windows x64. All three lanes
qualified commit `2b793c81e0987f627ab72e3c4e505ae5c6a95abe` with fresh external build,
evidence and artifact directories. The exact expanded commands are retained in
each lane's `qualification-transcript.txt`.

## MSVC

The Visual Studio 2022 Build Tools x64 environment supplied MSVC 19.44.35228,
CMake 3.31.6 and Ninja 1.12.1.

```powershell
$env:CC = 'cl.exe'
$env:CXX = 'cl.exe'
concurrency/qa/run-windows-qualification.ps1 `
  -ExpectedCommit 2b793c81e0987f627ab72e3c4e505ae5c6a95abe `
  -BuildDirectory C:\crexx-qa\windows-msvc-2b793c81e-build `
  -EvidenceDirectory C:\crexx-qa\windows-msvc-2b793c81e-evidence `
  -BuildJobs 2 -TestJobs 2
```

## Clang

The MSYS2 UCRT64 environment supplied Clang 22.1.7, CMake 4.3.3 and Ninja
1.13.2.

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
$env:CC = 'clang.exe'
$env:CXX = 'clang++.exe'
concurrency/qa/run-windows-qualification.ps1 `
  -ExpectedCommit 2b793c81e0987f627ab72e3c4e505ae5c6a95abe `
  -BuildDirectory C:\crexx-qa\windows-clang-2b793c81e-build `
  -EvidenceDirectory C:\crexx-qa\windows-clang-2b793c81e-evidence `
  -BuildJobs 2 -TestJobs 2
```

## GCC

The MSYS2 UCRT64 environment supplied GCC 16.1.0, CMake 4.3.3 and Ninja
1.13.2.

```powershell
$env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
$env:CC = 'gcc.exe'
$env:CXX = 'g++.exe'
concurrency/qa/run-windows-qualification.ps1 `
  -ExpectedCommit 2b793c81e0987f627ab72e3c4e505ae5c6a95abe `
  -BuildDirectory C:\crexx-qa\windows-gcc-2b793c81e-build `
  -EvidenceDirectory C:\crexx-qa\windows-gcc-2b793c81e-evidence `
  -BuildJobs 2 -TestJobs 2
```

The runner itself performs configure, build, maintained label execution, live
TLS verification, stress repetition, complete CTest, install, archive creation,
extraction, both applicable VM smokes, result generation and evidence digest
closure. No build or test invocation overlapped another invocation in the same
tree.
