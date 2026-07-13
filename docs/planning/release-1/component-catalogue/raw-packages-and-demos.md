# Raw packages, exits, tools, demos, examples, and PoCs

Stage 2 discovery data. CMake references are literal source facts, not delivery recommendations.

## RXPA plugin directories

| ID | Directory | C sources | cREXX/RXPP adapters | Local CMake | Root CMake reference |
|---|---|---:|---:|---|---|
| `PKG-RXPA-arrays` | `lib/plugins/arrays` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-cipher` | `lib/plugins/cipher` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-console` | `lib/plugins/console` | 1 | 1 | yes | commented `add_subdirectory` |
| `PKG-RXPA-fileio` | `lib/plugins/fileio` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-fpool` | `lib/plugins/fpool` | 0 | 0 | yes | no root reference |
| `PKG-RXPA-getpi` | `lib/plugins/getpi` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-gui` | `lib/plugins/gui` | 1 | 2 | yes | active `add_subdirectory` |
| `PKG-RXPA-id` | `lib/plugins/id` | 7 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-keyaccess` | `lib/plugins/keyaccess` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-llist` | `lib/plugins/llist` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-map` | `lib/plugins/map` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-matrix` | `lib/plugins/matrix` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-odbc` | `lib/plugins/odbc` | 1 | 2 | yes | active `add_subdirectory` |
| `PKG-RXPA-pick` | `lib/plugins/pick` | 1 | 1 | yes | commented `add_subdirectory` |
| `PKG-RXPA-pipe` | `lib/plugins/pipe` | 1 | 1 | yes | commented `add_subdirectory` |
| `PKG-RXPA-process` | `lib/plugins/process` | 1 | 3 | yes | commented `add_subdirectory` |
| `PKG-RXPA-recv390` | `lib/plugins/recv390` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-regex` | `lib/plugins/regex` | 1 | 1 | yes | commented `add_subdirectory` |
| `PKG-RXPA-rxmath` | `lib/plugins/rxmath` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-rxml` | `lib/plugins/rxml` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-rxtcp` | `lib/plugins/rxtcp` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-socket` | `lib/plugins/socket` | 1 | 2 | yes | active `add_subdirectory` |
| `PKG-RXPA-stack` | `lib/plugins/stack` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-strings` | `lib/plugins/strings` | 1 | 1 | yes | active `add_subdirectory` |
| `PKG-RXPA-system` | `lib/plugins/system` | 1 | 5 | yes | active `add_subdirectory` |
| `PKG-RXPA-testrx` | `lib/plugins/testrx` | 0 | 0 | no | no root reference |
| `PKG-RXPA-treemap` | `lib/plugins/treemap` | 2 | 1 | yes | active `add_subdirectory` |

## VM plugin directories

| ID | Directory | C sources | Build evidence |
|---|---|---:|---|
| `PKG-RXVM-db-decimal` | `interpreter/rxvmplugin/rxvmplugins/db_decimal` | 2 | named in `interpreter/rxvmplugin/rxvmplugins/CMakeLists.txt` |
| `PKG-RXVM-mc-decimal` | `interpreter/rxvmplugin/rxvmplugins/mc_decimal` | 23 | named in `interpreter/rxvmplugin/rxvmplugins/CMakeLists.txt` |

## Compiler exits

| ID | Exit | Namespace export | Source | Bundle evidence |
|---|---|---|---|---|
| `EXIT-add` | `add` | `rxcpexits.addexit` | `compiler/exits/add/Add.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-address` | `address` | `rxcpexits.addressexit` | `compiler/exits/address/Address.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-dummy` | `dummy` | `rxcpexits.dummyexit` | `compiler/exits/dummy/Dummy.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-dump` | `dump` | `rxcpexits.dumpexit` | `compiler/exits/dump/Dump.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-execio` | `execio` | `rxcpexits.execioexit` | `compiler/exits/execio/Execio.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-fsay` | `fsay` | `rxcpexits.fsayexit` | `compiler/exits/fsay/Fsay.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-msay` | `msay` | `rxcpexits.msayexit` | `compiler/exits/msay/Msay.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-parse` | `parse` | `rxcpexits.parseexit` | `compiler/exits/parse/Parse.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-pprint` | `pprint` | `rxcpexits.pprintexit` | `compiler/exits/pprint/Pprint.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-query` | `query` | `rxcpexits.queryexit` | `compiler/exits/query/Query.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-rexxscript` | `rexxscript` | `rxcpexits.rexxscriptexit` | `compiler/exits/rexxscript/RexxScript.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-signal` | `signal` | `rxcpexits.signalexit` | `compiler/exits/signal/Signal.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-sort` | `sort` | `rxcpexits.sortexit` | `compiler/exits/sort/Sort.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-sortx` | `sortx` | `rxcpexits.sortexit2` | `compiler/exits/sortx/Sortx.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-substr` | `substr` | `rxcpexits.substrexit` | `compiler/exits/substr/Substr.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |
| `EXIT-trace` | `trace` | `rxcpexits.traceexit` | `compiler/exits/trace/Trace.crexx` | directory is active from `compiler/exits/CMakeLists.txt` |

## Tools and product modules

| ID | Component | Primary sources | Build evidence |
|---|---|---|---|
| `TOOL-rxc` | typed/Level C compiler and parser service | `compiler` | `compiler/CMakeLists.txt` builds `rxc` |
| `TOOL-rxas` | RXAS assembler | `assembler` | `assembler/CMakeLists.txt` builds `rxas` |
| `TOOL-rxlink` | RXBIN linker | `linker` | `linker/CMakeLists.txt` builds `rxlink` |
| `TOOL-rxdas` | RXBIN disassembler | `disassembler` | `disassembler/CMakeLists.txt` builds `rxdas` |
| `TOOL-rxcpack` | RXBIN-to-C packager | `cpacker` | `cpacker/CMakeLists.txt` builds `rxcpack` |
| `TOOL-crexx` | high-level compile/assemble/run/native driver | `bin/crexx.crexx` | `bin/CMakeLists.txt` builds `crexx` |
| `TOOL-rxvm` | threaded bare VM executable | `interpreter` | `interpreter/CMakeLists.txt` builds `rxvm` |
| `TOOL-rxbvm` | bytecode-dispatch bare VM executable | `interpreter` | `interpreter/CMakeLists.txt` builds `rxbvm` |
| `TOOL-rxvme` | threaded VM with packed Level B library | `interpreter` | `interpreter/CMakeLists.txt` builds `rxvme` |
| `TOOL-rxbvme` | bytecode-dispatch VM with packed Level B library | `interpreter` | `interpreter/CMakeLists.txt` builds `rxbvme` |
| `LIB-rxvml` | threaded embeddable VM static library | `interpreter/rxvmlib.c; interpreter/rxvml.c` | `interpreter/CMakeLists.txt` builds `rxvml` |
| `LIB-rxbvml` | bytecode-dispatch embeddable VM static library | `interpreter/rxvmlib.c; interpreter/rxvml.c` | `interpreter/CMakeLists.txt` builds `rxbvml` |
| `LIB-crexxsaa` | public SAA-style host integration shared library/API | `interpreter/crexxsaa.c; interpreter/crexxsaa.h` | explicitly built and installed by `interpreter/CMakeLists.txt` |
| `TOOL-crexxsaa` | CREXXSAA cache maintenance CLI | `interpreter/crexxsaa_tool.c` | explicitly built and installed by `interpreter/CMakeLists.txt` |
| `INFRA-RXPA` | native plugin ABI/build framework | `rxpa; include/crexxpa.h; lib/include/rxpa.h` | `rxpa/CMakeLists.txt` and plugin target helpers |
| `INFRA-RXPASHIM` | RXPA symbol shim for standalone/native targets | `interpreter/rxpashim.c` | static target in `interpreter/CMakeLists.txt` |
| `TOOL-rxpp` | RXPP preprocessor | `preprocessor/rxpp.crexx; preprocessor/precomp.c` | `preprocessor/CMakeLists.txt` |
| `TOOL-rxpp-sh` | optional RXPP parser/source-map shell | `preprocessor/rxpp_sh.c` | built when parser mode is enabled |
| `PKG-RXPA-precomp` | RXPP internal native primitive module | `preprocessor/precomp.c` | static and dynamic RXPA targets in `preprocessor/CMakeLists.txt` |
| `LIB-RXPP-maclib` | staged RXPP standard macro definitions | `preprocessor/maclib.rexx` | staged, built as an `ALL` dependency, and installed by `preprocessor/CMakeLists.txt` |
| `LIB-RXPP-macsys` | staged RXPP system-oriented macro definitions | `preprocessor/macsys.rexx` | staged, built as an `ALL` dependency, and installed by `preprocessor/CMakeLists.txt` |
| `LIB-RXPP-mathlib` | staged RXPP `##USE` mathematical procedures | `preprocessor/mathlib.rexx` | staged, built as an `ALL` dependency, and installed by `preprocessor/CMakeLists.txt` |
| `LIB-RXPP-syslib` | staged empty RXPP `##USE` placeholder | `preprocessor/syslib.rexx` | staged, built as an `ALL` dependency, and installed by `preprocessor/CMakeLists.txt` |
| `TOOL-rexxscript` | RexxScript runtime and CLI | `rexxscript/*.crexx` | `rexxscript/CMakeLists.txt` |
| `TOOL-rxdb` | RXDB debugger | `debugger/rxdb.crexx; debugger/rxdb_gui.crexx` | `debugger/CMakeLists.txt` |
| `INFRA-VM-PROFILING` | compile-time-gated VM timing and dynamic-sequence profiling | `interpreter/rxvmprofile.*; interpreter/rxvminstrument_profile.h; interpreter/rxvmsequence.c` | `CREXX_VM_PROFILING` paths in `interpreter/CMakeLists.txt` |
| `TOOL-rxseq` | offline dynamic instruction-sequence profile analyser | `interpreter/rxseqmain.c; interpreter/rxseqfile.c` | `interpreter/CMakeLists.txt` builds `rxseq` |
| `TOOL-rxvm-instrumented` | threaded instrumentation-contract test VM | instrumented VM object variant | built only with `BUILD_TESTING` |
| `TOOL-rxbvm-instrumented` | bytecode-dispatch instrumentation-contract test VM | instrumented VM object variant | built only with `BUILD_TESTING` |
| `LIB-rxmath-source` | older rxmath source tree | `lib/rxmath/rexx; lib/rxmath/native; lib/rxmath/rxas` | `no lib/rxmath CMakeLists.txt found` |
| `LIB-veclib` | vector library | `lib/veclib/veclib.crexx` | `lib/veclib/CMakeLists.txt` |

## Residual and alternate implementation source candidates

| ID | Candidate | Source | Literal build/source fact |
|---|---|---|---|
| `SRC-RXMATH-cos` | `cos` routine candidate | `lib/rxmath/rexx/cos.rexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-fact` | `fact` routine candidate | `lib/rxmath/rexx/fact.crexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-gcd` | `gcd` routine candidate | `lib/rxmath/rexx/gcd.crexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-lcm` | `lcm` routine candidate | `lib/rxmath/rexx/lcm.crexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-ln` | `ln` routine candidate | `lib/rxmath/rexx/ln.rexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-ln10` | `ln10` routine candidate | `lib/rxmath/rexx/ln10.rexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-ln2` | `ln2` routine candidate | `lib/rxmath/rexx/ln2.rexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-ln2p` | `ln2p` routine candidate | `lib/rxmath/rexx/ln2p.rexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-pi` | `pi` routine candidate | `lib/rxmath/rexx/pi.rexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-piconst` | `piconst` routine candidate | `lib/rxmath/rexx/piconst.crexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-sin` | `sin` routine candidate | `lib/rxmath/rexx/sin.rexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXMATH-sqrt` | `sqrt` routine candidate | `lib/rxmath/rexx/sqrt.rexx` | no `lib/rxmath/CMakeLists.txt` exists |
| `SRC-RXPP-rxppt` | older RXPP implementation/workbench candidate | `preprocessor/rxppt.crexx` | contains machine-specific paths and is not named by `preprocessor/CMakeLists.txt` |
| `SRC-RXFNSB-RXAS-elapsed-rxas` | RXAS routine/source candidate | `lib/rxfnsb/rxas/_elapsed.rxas` | active selector/import dependency |
| `SRC-RXFNSB-RXAS-rxvml-address-native-rxas` | RXAS routine/source candidate | `lib/rxfnsb/rxas/_rxvml_address_native.rxas` | active selector/import dependency |
| `SRC-RXFNSB-RXAS-copies-rxas-depircated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/copies.rxas.depircated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-dectest-rxas` | RXAS routine/source candidate | `lib/rxfnsb/rxas/dectest.rxas` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-lastpos-rxas` | RXAS routine/source candidate | `lib/rxfnsb/rxas/lastpos.rxas` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-left-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/left.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-length-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/length.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-pos-rxas` | RXAS routine/source candidate | `lib/rxfnsb/rxas/pos.rxas` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-reverse-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/reverse.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-right-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/right.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-substr-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/substr.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-subword-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/subword.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-testbifs-rxas` | RXAS routine/source candidate | `lib/rxfnsb/rxas/testbifs.rxas` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-word-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/word.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-wordindex-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/wordindex.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-wordlen-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/wordlen.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-wordlength-rxas-depricated` | RXAS routine/source candidate | `lib/rxfnsb/rxas/wordlength.rxas.depricated` | present in source; not an active library selector |
| `SRC-RXFNSB-RXAS-words-rxas` | RXAS routine/source candidate | `lib/rxfnsb/rxas/words.rxas` | present in source; not an active library selector |
| `PKG-RXFNSB-NATIVE-DES` | native DES function source | `lib/rxfnsb/native/des` | parent `lib/rxfnsb/CMakeLists.txt` does not add `native` |
| `INFRA-EXIT-TOKEN` | compiler-exit token/plan contract | `compiler/exits/token.crexx` | built into `rxcexits.rxbin` as exit infrastructure |

## Demonstrations, examples, and compiler PoCs

| ID | Kind | Source |
|---|---|---|
| `DEMO-demos-cms-cms-address-demo-crexx` | demo | `demos/cms/cms_address_demo.crexx` |
| `DEMO-demos-cms-cms-address-environment-crexx` | demo | `demos/cms/cms_address_environment.crexx` |
| `DEMO-demos-llm-anthropic-generate-crexx` | demo | `demos/llm/anthropic_generate.crexx` |
| `DEMO-demos-llm-gemini-generate-crexx` | demo | `demos/llm/gemini_generate.crexx` |
| `DEMO-demos-llm-llm-address-demo-crexx` | demo | `demos/llm/llm_address_demo.crexx` |
| `DEMO-demos-llm-llm-address-environment-crexx` | demo | `demos/llm/llm_address_environment.crexx` |
| `DEMO-demos-llm-ollama-generate-crexx` | demo | `demos/llm/ollama_generate.crexx` |
| `DEMO-demos-llm-ollama-generate-rxas` | demo | `demos/llm/ollama_generate.rxas` |
| `DEMO-demos-llm-openai-generate-crexx` | demo | `demos/llm/openai_generate.crexx` |
| `DEMO-demos-native-kv-kv-address-demo-crexx` | demo | `demos/native/kv/kv_address_demo.crexx` |
| `DEMO-demos-native-sqlite-sqlite-address-demo-crexx` | demo | `demos/native/sqlite/sqlite_address_demo.crexx` |
| `DEMO-demos-rexxscript-rexxscript-demo-crexx` | demo | `demos/rexxscript/rexxscript_demo.crexx` |
| `EXAMPLE-examples-montecarlo-crexx` | example | `examples/MonteCarlo.crexx` |
| `EXAMPLE-examples-anagram-crexx` | example | `examples/anagram.crexx` |
| `EXAMPLE-examples-caesar-crexx` | example | `examples/caesar.crexx` |
| `EXAMPLE-examples-date1-crexx` | example | `examples/date1.crexx` |
| `EXAMPLE-examples-exposevars-crexx` | example | `examples/exposeVars.crexx` |
| `EXAMPLE-examples-factorial-crexx` | example | `examples/factorial.crexx` |
| `EXAMPLE-examples-fibonacci-crexx` | example | `examples/fibonacci.crexx` |
| `EXAMPLE-examples-gausssqr-crexx` | example | `examples/gausssqr.crexx` |
| `EXAMPLE-examples-gcd-crexx` | example | `examples/gcd.crexx` |
| `EXAMPLE-examples-hello-crexx` | example | `examples/hello.crexx` |
| `EXAMPLE-examples-lcm-crexx` | example | `examples/lcm.crexx` |
| `EXAMPLE-examples-nthroot-crexx` | example | `examples/nthroot.crexx` |
| `EXAMPLE-examples-oobank-crexx` | example | `examples/ooBank.crexx` |
| `EXAMPLE-examples-sieveeratosthenes-crexx` | example | `examples/sieveEratosthenes.crexx` |
| `EXAMPLE-examples-sort-crexx` | example | `examples/sort.crexx` |
| `EXAMPLE-examples-stems-rxpp` | example | `examples/stems.rxpp` |
| `EXAMPLE-examples-strings1-crexx` | example | `examples/strings1.crexx` |
| `EXAMPLE-examples-system-crexx` | example | `examples/system.crexx` |
| `EXAMPLE-examples-time-sample-crexx` | example | `examples/time_sample.crexx` |
| `EXAMPLE-examples-xref-crexx` | example | `examples/xref.crexx` |
| `POC-levelc-lemon-fallback` | compiler PoC | `compiler/pocs/levelc_lemon_fallback` |
| `POC-levelc-token-adapter-trace` | compiler PoC | `compiler/pocs/levelc_token_adapter_trace` |
