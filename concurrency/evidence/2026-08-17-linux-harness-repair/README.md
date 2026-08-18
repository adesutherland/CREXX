# Linux concurrency harness repair validation

Date: 2026-08-17
Branch: `develop`
Parent source commit: `c38a5d184dee1ecd659c83ccbc4a18001d46dc77`
Candidate: the commit containing this evidence directory

Status: **local Linux harness repair validation PASS.** The exact-commit QA-C
runner must be replayed from the clean candidate commit before it can issue the
formal platform `RESULT.txt` and closed evidence digest.

## Repair result

The rotating `ts_http_server_*` timeout was cross-test oversubscription, not a
VM, endpoint, socket or HTTP server implementation defect. Each server case
creates several task pools inside one VM, but CTest previously overlapped those
VMs with compiler/runtime tests which launch more child processes. A client
could exhaust its bounded deadline under that load, leaving the documented
request-count-bounded server waiting for a request that would never arrive.

All eight positive and negative server tests now use CTest `RUN_SERIAL`. Their
parallel clients and task handlers remain unchanged inside each VM.

Two rotating syntax-highlighting assertions then exposed the same host-load
class in the complete sweep. These tests take asynchronous parser snapshots.
Their existing parser `RESOURCE_LOCK` is retained, and the test factory now
also uses `RUN_SERIAL` so snapshots do not compete with CPU-heavy child
processes. No compiler implementation changed.

## Validation result

- the four-job current-tree build completed;
- the eight server cases passed five repetitions, 40 VM/mode executions total;
- the maintained concurrency matrix passed 183/183 in 133.05 seconds at
  `--parallel 10`;
- all four live TLS VM/mode cases passed trusted-host verification and rejected
  the hostname-mismatch host;
- the six concurrency stress tests passed 20 repetitions, 120 executions total;
- the syntax-highlighting matrix passed 72/72 in 55.63 seconds;
- complete CTest passed 2,207/2,207 in 949.84 seconds;
- the concurrency umbrella and every SP-01 through SP-09 label were non-empty;
- a fresh external install ran the documented basic concurrency example on
  installed `rxbvm` and `rxtvm`;
- the portable ZIP passed its integrity test;
- the Debian package built and the extracted package ran the same example on
  both VMs; and
- `library.rxbin`, `classlib.rxbin` and `rxfnsg.rxbin` were present in the
  packaged payload.

No `rxbvm`, `rxtvm`, CTest, CMake or Ninja process remained after validation.

## Evidence map

- `qa/build.log`: four-job current-tree build;
- `qa/server-repeat-5.log`: five repetitions of every server VM/mode case;
- `qa/concurrency-matrix.log`: maintained 183-test concurrency matrix;
- `qa/tls-live.log`: live trusted-host and mismatch-host proof;
- `qa/stress-repeat-20.log`: 20 unchanged stress repetitions;
- `qa/syntax-highlighting-matrix.log`: isolated 72-test parser matrix;
- `qa/full-ctest.log`: final 2,207-test complete regression sweep;
- `qa/label-counts.txt`: concurrency umbrella and SP label counts;
- `install/`: installed inventory and both-VM smoke;
- `package/`: Debian build/smoke, payload inventory and ZIP/DEB digests; and
- [`COMMANDS.md`](COMMANDS.md): replay commands and qualification boundary.

The ZIP and Debian binaries are retained outside the repository. Their digests
are committed in `package/packages.sha256`.
