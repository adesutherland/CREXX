# Native allocation and export amendment

11 October 2026. This supersedes the failed heap256 candidate for execution;
the original candidate and its receipts remain evidence. Guest script
qualification is open. No release or managed PDOS adoption is implied.

Real CMS20 assembled the unchanged Lab `tests/cms31/storage.asm` through
ASMAHL and its DMSGPI/DMSOM macros. Independent review confirms the SDK
obtain/release lists, R0 byte units, R15=00E00000 conditional-return flags and
32-byte list extent. There is no fence in this old expansion. The private
listing SHA-256 is
`7154eca4946132c26d5574b7bbadf112759cf65d264253759c93f9d308b7df3f`.
This is assembly evidence, not execution of a new storage probe. The modern
IBM STPL mapping was a useful comparison lead, not this guest's ABI authority.

Native RXC with a fixed 256 MiB heap failed its first storage obtain at 512M
virtual storage during `-v`, before any ADDRESS or library import. Its following
protection exception maps by the DMSFRO caller and image map to rxpa_addclass.
Source confirms unchecked compiler RXPA metadata/static-function allocations;
the dynamic-function path also silently dropped allocation failure. All three
now use the existing allocation-free panic and exit path. Deterministic tests
exercise all five metadata/function callbacks in both registration contexts,
verify no partial list insertion on failure, and retain healthy registration.
This repairs the failure response; it does not make rejected storage available.

The native capacity oracle at recorded 512M obtained, checked the first and
last bytes, and released serial 64, 128 and 192 MiB blocks. The 256 MiB obtain
was unavailable, with final native RC 4 and successful QUERY DISK recovery.
It ran without C/newlib/cREXX. Its source SHA-256 is
`275d792e551c772d5b1a11df01d677d9d99aad19cd180d612665a7101ed0d667`;
the reviewed native MODULE stream SHA-256 is
`eb2e57a0e649fcc37d458d5d33e420b5c9dc04c83cd18ceb319bc9953e3e9bb7`.
This proves these four outcomes, not the largest free block or availability
beside a larger compiler image. Original 64M was restored at normal handback.

The selected CMS candidate requests 192 MiB at 512M virtual storage, retains
full current G and Classic imports, and includes the compiler allocation repair.
The SDK overlay replaces the known cms-c2.o member; default/released profiles
are unchanged. All four CMS tools are rebuilt. Actual heap sufficiency and
high-water must be observed. Host matching-import Level B compilation succeeds
with 233,570,304 bytes maximum RSS and 226,738,800 bytes peak memory footprint;
those 64-bit macOS measures are not a 31-bit native heap measurement.

All four TSO tools are rebuilt with a 192 MiB application heap and the repaired
producer. Its native TSO region and full scripts remain to be qualified.
The retained child/fault controls keep their independently pinned profiles.
Classic-ld derives
the native directory member from its output argument; passing the old absolute
path yielded `/USERS/A`. The producer now passes a basename from the output
directory and requires the SDK's independent XMIT reader to verify the exact
tool name, AMODE 31 and RMODE ANY. The old RXC export is explicitly rejected by
that reader. All four new exports pass. The reader is included in the producer
before/after source and tool hash set. RXC includes the allocation repair;
successful-program behavior and ADDRESS semantics are unchanged.

The new host-check directory also omitted QA tier finalization, leaving its
declared bytecode prerequisites outside qa-prep. The full macOS ASan run used
old Level B bytecode and returned 19 for both caller-file checks, without a
sanitizer memory diagnostic. Finalization is now connected and the affected
fixtures are rebuilt from the complete four-library guest collection with -n.
Fresh Debug and macOS ASan pass all 11 current ADDRESS/allocation checks.
Preserve the failed full run and keep final full qualification distinct.

The complete replacement pack must use `mainframe-qa-pack.crexx` arguments
`allocationfix memberfix`, preserve all source/tool before-and-after receipts,
and pin the current scripts, libraries, PDOS prerequisite, operator helpers
and both reviewed amendments before staging. Run the four real/native-emulated
script cells, including caller-file survival, RC/conditions, supported capture,
selected fault recovery and normal fleet handback. The native CMS arbitrary
child-ABEND boundary remains unqualified. The PDOS large-file profile is
disposable; managed images are unchanged.
