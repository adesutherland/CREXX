# PERF3-13 E5 Windows 11 combined-PoC handoff

This directory records the source-complete handoff of the private E5 executor
PoC after Windows 11 qualification. It complements the retained macOS and
Linux evidence; it does not authorize an industrial mailbox, public worker API,
E6 or Gate F.

## Combined source shape

One executor and one test harness are shared by all supported platforms:

- `interpreter/rxvmexecutor.c` owns the immutable per-executor capability
  choice and the persistent worker lifecycle;
- `interpreter/interrupt.c` contains the Apple/Linux `SIGURG` carrier and the
  dynamically resolved Windows special-APC carrier;
- `interpreter/rxvmintp.c` contains the unchanged ordinary E4 owner and the
  duplicate sparse compatibility owner for switch and computed-goto engines;
- `interpreter/rxvmhandlers_*.inc` and `interpreter/rxvmhandlerpolicy.h` keep
  both owners on the same handler semantics and inline/outline policy;
- `interpreter/tests/test_persistent_worker_executor.c` supplies ordinary,
  stress, native-doorbell, latency, fallback and sparse-progress modes;
- `interpreter/tests/rexx/test_persistent_worker_executor.crexx` supplies the
  normal loop/recursion/reuse fixture; and
- `interpreter/tests/test_persistent_worker_sparse.rxas` supplies exact
  conditional, counted and indirect backedge coverage plus all five bytecode
  return forms.

The RXAS sparse fixture is a new source file and must be included in the commit.
No generated RXBIN is required in source control. CMake regenerates both
fixtures by default, or accepts known-qualified binaries through
`CREXX_E5_TEST_RXBIN` and `CREXX_E5_SPARSE_TEST_RXBIN` to avoid recompiling the
fixture toolchain in additional compiler lanes.

## Runtime selection

- macOS/Linux targetable worker: install the existing private POSIX native
  signal carrier and retain the ordinary E4 owner;
- Windows targetable worker with a successful `QueueUserAPC2` special-APC
  probe: retain the ordinary E4 owner;
- Windows targetable worker when resolution/installation fails: select the
  sparse compatibility owner before `rxvm_prepare()`; and
- non-targetable/local context: retain the ordinary E4 owner.

An executing context never changes owner, and worker migration does not cause
rethreading. The Windows forced-unavailable test environment variable is
`CREXX_VM_POC_WINDOWS_APC_FORCE_UNAVAILABLE=1` and is meaningful only with
`CREXX_VM_POC_DOORBELL=windows-special-apc`.

## Windows qualification result

| Compiler lane | Build | Focused result | Concrete owners |
| --- | --- | ---: | --- |
| MinGW GCC | ordinary profiling-off Release | 19/19 | switch and computed goto |
| MSVC 19.44 | Debug | 13/13 | switch |
| Clang 22.1.8, MSVC ABI | Debug | 19/19 | switch and computed goto |

All lanes build `rxc`, `rxas`, `rxlink` and the configured `rxvm`. Each real
`rxc.exe` compiles the former access-violation fixture in optimized and no-opt
modes. The MSVC and Clang/MSVC-ABI lanes use `ENABLE_PARSER_MODE=OFF` because
the optional sibling `DSL-Syntax-Highlighter` still includes POSIX
`unistd.h`; MinGW GCC validates the default parser-enabled configuration.

The access violation was an omitted initialization of
`active.compatibility_interrupts`, not a Windows security feature.
`rxinimod_common()` now initializes it to NULL, and `test_rxvmactive` poisons
the context before initialization to guard the embedding contract.

The retained sparse-fallback latency is 2.9-3.0 microseconds median over 1,000
cancellations per concrete engine. Retained 12-pair targetable-fallback costs
are +16.09% mean/+14.89% median for `rxbvml` and +5.69%/+5.07% for `rxtvml`.
These figures do not apply to non-targetable or native-capable execution, which
continues to use E4. Duplicate-owner product growth is about 19.4-20.0% and was
accepted for the PoC.

## Continuation boundaries

- macOS/Linux should rebuild and rerun the existing native-doorbell panels
  after consolidation; Windows-only forced fallback is not registered as a
  POSIX CTest;
- native/plugin return contains a sparse observation point, but a dedicated
  runtime fixture for that return edge remains follow-on work;
- a native/plugin call that does not return remains outside cooperative
  cancellation;
- the branch-independent compiler/CMake Windows portability edits may be
  separated onto `develop`; the compatibility-pointer initializer and handler
  owner changes must remain with `mthread`; and
- build directories and root-level `w1`-`w9` diagnostic logs are local scratch
  material and are not required by the continuation commit.

See `COMMANDS.md` for reproducible build and focused-test entry points.
