# RXBIN repair receipts

Baseline product: `143921e11e4d573909fcc4def28da5dceadba9d9`.

The legacy ILP32 import mutation in `test_compact_format` fails against the
baseline library: `Imported procedure start was not normalized`, status 1
([log](sentinel-before.log)). Both all-ones u64 and old imported all-ones u32
now normalize to native SIZE_MAX; rewriting each verifies canonical u64 output.
The same test rejects a non-sentinel address exceeding SIZE_MAX in ILP32 builds.

`tools/check-rxbin-width.sh "$PWD/cmake-build-debug/bin/rxas"
"$PWD/cmake-build-debug/beta3-standalone"` passes locally on LP64. The script
compiles only the portable binary library plus the format test, consuming a
native-RXAS-generated fixture. Actual Linux ILP32 execution with gcc-multilib
and `CFLAGS=-m32` passed in Build run `36440639967`, Core linux-x64 job
`108990065887`, on published head `a4a39dc3b9c63fff9c726842f1c8ea43ded9326e`:
`PASS RXBIN cross-width reader/writer (gcc -m32)`. The retained runner log is
`/var/folders/nr/7ckzqpl91kz80mcy3316h1tr0000gn/T/crexx-beta3-linux-retry.n2Iuq6YKeU`,
SHA-256 `7b4bc33100119f61279f2e7adee99e7365bf8ff41b0104f869b05eb79960c908`.
This proves the portable RXBIN reader/writer on an actual 32-bit Linux process,
not a native CMS/TSO package.

The production compressor is compiled directly into the fault-injection unit
without a replacement algorithm. Deterministic mixed/random input of 20,000
bytes gives the same 12,207 bytes and FNV-1a digest `3fc9210d4fbd2dba` before/after.
The baseline has one workspace allocation, the repaired version two. The test
rejects failures of either workspace allocation and early/late output writes,
checks all workspace frees, an empty input and full decompression equality.
[Candidate output](compression-after.log).

Apple Clang `cc -std=c99 -fstack-usage -I binutils/include -I platform
-I assembler -c binutils/rxbin007.c` reports compressor stack use of 176 bytes,
versus 65,728 bytes for the baseline source compiled with identical options.
The moved table requires 65,536 heap bytes on LP64 or 32,768 on ILP32; total
per-call workspace is 98,312 / 49,156 bytes respectively. Algorithm and format
are unchanged. This measures host compiler frames, not mainframe native stack
qualification.
