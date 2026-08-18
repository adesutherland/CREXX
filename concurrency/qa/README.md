# Initial concurrency platform qualification pack

These runners execute QA-C on Linux and QA-D on Windows. They validate one
exact clean commit in fresh out-of-tree directories; they are not development
scripts and never edit the checkout.

## Required invocation

Linux:

```sh
concurrency/qa/run-linux-qualification.sh \
  EXPECTED_FULL_COMMIT BUILD_DIRECTORY EVIDENCE_DIRECTORY
```

Windows PowerShell:

```powershell
concurrency/qa/run-windows-qualification.ps1 \
  -ExpectedCommit EXPECTED_FULL_COMMIT \
  -BuildDirectory BUILD_DIRECTORY \
  -EvidenceDirectory EVIDENCE_DIRECTORY
```

The build and evidence directories must be empty and outside the repository.
The checkout must be detached at, or have `HEAD` equal to, the expected
40-character commit with no tracked, staged or untracked changes.

Both runners:

1. record source, host, compiler, CMake, generator and TLS-backend identity;
2. configure a fresh distribution-shaped `MinSizeRel` Ninja build;
3. complete the ordinary product build;
4. run the maintained `concurrency-qa` target with live trusted-host and
   hostname-mismatch TLS verification enabled;
5. prove every SP-01 through SP-09 label is non-empty;
6. repeat the six stress tests 20 times;
7. run the complete CTest suite without overlapping build/test processes;
8. install to a fresh prefix and run the documented Level G basic example
   through installed `rxc`, `rxas`, `rxlink` and every supported concrete VM;
9. build and validate the platform's portable package, plus the Linux `.deb`;
   and
10. re-run the same installed-toolchain smoke from the extracted package and
    finish only if the checkout remains clean.

The live HTTP/TLS test defaults to `example.com` and
`wrong.host.badssl.com`. Override them with
`CREXX_HTTP_TLS_LIVE_HOST` and `CREXX_HTTP_TLS_MISMATCH_HOST` when the
qualification network provides controlled equivalents. A network block or
certificate-policy failure is retained as a qualification failure, not turned
into a skip.

## Expected result rules

- every command exits zero;
- every selected CTest execution passes and CTest reports no missing tests;
- each of `concurrency-sp01` through `concurrency-sp09` selects at least one
  test;
- live TLS reports a successful trusted-host request and rejection of the
  hostname-mismatch host on every supported concrete VM/mode;
- the 20-cycle stress command has no timeout, crash, race diagnostic or
  failure;
- installed and extracted-package toolchain smokes print
  `PASS: basic concurrency example` for `rxbvm` and for `rxtvm` where that VM
  is supported;
- package integrity checks pass and the inventory contains `library.rxbin`,
  `classlib.rxbin` and `rxfnsg.rxbin`; and
- `RESULT.txt` says `PASS` and names the exact commit.

Counts can differ because direct-threaded `rxtvm` is compiler/platform
conditional. A smaller count is acceptable only when the configure log names
that unsupported variant and all tests registered for the resulting build
pass. Do not edit source on the qualification host to change a result.

On failure, retain the complete evidence directory, record the first failing
command and exact test, and return the defect to Mac with a minimal reproducer.
After a Mac repair and local QA, replay the whole platform runner at the new
exact commit.

The output layout is described in [`EVIDENCE.md`](EVIDENCE.md).
