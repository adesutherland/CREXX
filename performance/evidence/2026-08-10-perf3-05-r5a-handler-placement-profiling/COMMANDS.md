# Replay boundary

The GCC profile-20 build used:

```sh
cmake -S . -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DCMAKE_C_COMPILER=/opt/homebrew/opt/gcc/bin/gcc-16 \
  -DCREXX_ENABLE_TLS=OFF -DCREXX_VM_PROFILING=ON \
  -DCREXX_VM_HANDLER_PANEL=profile-20
cmake --build "$BUILD" --target rxtvm rxbvm --parallel 2
```

The exact counts-only capture for each concrete engine was:

```sh
"$VM" --profile=counts --profile-output="$PROFILE" \
  benchmark_awfy_bounce_opt.rxbin library.rxbin -a 4200
```

The RXBIN and library are the same frozen R3/R5 inputs used by the retained
formal comparison. Output must contain `PASS: AWFY Bounce`; stderr must be
empty. Profile timing fields are zero and are not benchmark evidence.

The focused profile tests were:

```sh
ctest --test-dir "$CLANG_PROFILE_BUILD" --parallel 1 --output-on-failure -R \
'^(rxvmprofileaccounting|rxvmprofiletable|rxbvmprofiletable|rxvmprofilecsv|rxbvmprofilecsv|vmprofilingdocumentation)$'
```

The focused ordinary instrumentation tests were:

```sh
ctest --test-dir "$CLANG_UNPROFILED_BUILD" --parallel 1 --output-on-failure -R \
'^(rxvminstrumentedsignaltests|rxbvminstrumentedsignaltests|rxvminstrumentedbreakpointtests|rxbvminstrumentedbreakpointtests)$'
```

For the ordinary expansion proof, the exact Clang profile-20 `rxvmintp.c`
compile flags were changed to `-E -P` against commit
`ab613c5c0593d58e696bbee1f5c8a5390f092d3a` and the working tree. Absolute
source roots were normalized to `/SOURCE`, then `cmp` passed.
