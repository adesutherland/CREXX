# DECIMAL-01 libmpdec candidate

This directory contains an isolated, non-default cREXX decimal plugin used to
screen libmpdec. It does not vendor libmpdec and is excluded from ordinary
builds.

The screened upstream input is mpdecimal 4.0.1 from the official archive,
whose SHA-256 digest is:

```text
96d33abb4bb0070c7be0fed4246cd38416188325f820468214471938545b1ac8
```

The upstream library is BSD-2-Clause licensed. Source and builds remain in an
external scratch directory during the candidate screen.

Configure the candidate explicitly, pointing at an already configured and
built upstream tree:

```text
cmake -S . -B cmake-build-release-mpd -DCMAKE_BUILD_TYPE=Release \
  -DCREXX_VM_PROFILING=OFF \
  -DCREXX_BUILD_MPD_DECIMAL_CANDIDATE=ON \
  -DLIBMPDEC_ROOT=/absolute/path/to/mpdecimal-4.0.1
cmake --build cmake-build-release-mpd --target \
  mpd_decimal_dynamic decimal_gate1_adapter
```

The plugin stores a pointer-free header and inline coefficient words in the
VM-owned decimal sidecar. Temporary `mpd_t` views point into that payload only
for the duration of a call. This preserves the existing raw-copy decimal
lifecycle without adding copy or destructor hooks to the plugin ABI.

The candidate is a rejected experiment, not a selectable production provider.
Its formal L1 adapter screen and the follow-up direct-core attribution are
retained in
[`2026-08-18-decimal-01-libmpdec-screen`](../../../evidence/2026-08-18-decimal-01-libmpdec-screen/).
Arithmetic and conversion failed the progression gate, so no L2/L3 or
production integration was run.
