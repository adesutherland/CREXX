# VM library link diagnostic

This is a read-only, isolated diagnosis of the report that linking against the
VM library is slow. It changes no production source or CMake target.

## Answer

Private-symbol exposure exists, but it is not the measured cause of current
Mac link time.

1. The static `rxvml` phase API is in one 5,904-byte object. Even
   create/destroy-only use loads that object, its reference to `run`, the
   805,296-byte interpreter object and the remaining implementation graph.
2. The shared `crexxsaa` target exposes implementation archives and internal
   include directories as `PUBLIC`, so downstream CMake links scan archives
   that should not be on the consumer interface.
3. The dylib exports 367 globals instead of only its 16 intended prefixed API
   symbols. Filtering to 16 exports reduces 6,920 bytes but does not change the
   15-run median relink time (37.392 ms unfiltered, 37.439 ms filtered).
4. Removing the propagated archives from the downstream command reduces its
   median from 35.095 ms to 21.706 ms. `saa-expanded-why-load.log` is empty,
   proving the extra archives are scanned but contribute no members.

Thus the actionable causes are API-object granularity and CMake link-interface
leakage. Export control is still correct ABI hygiene, but it is not a current
link-speed fix. The original Ninja times—61, 64 and 71 ms for the three
relevant links—also do not reproduce a large Mac linker bottleneck.

Rework should be separately approved and should validate:

- `PRIVATE` implementation dependencies for `crexxsaa`;
- a public build/install include interface containing only supported headers;
- an export list or hidden-by-default visibility on every supported platform;
- splitting lifecycle/load/link/prepare/call wrappers so a narrow static use
  does not reference `run()` unless execution is requested; and
- the reported slow host and client before/after, since the current Mac does
  not reproduce a materially slow link.

