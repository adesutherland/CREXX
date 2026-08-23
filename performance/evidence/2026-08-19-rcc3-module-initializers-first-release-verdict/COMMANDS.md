# Commands

The existing profiling-off Release products were frozen before rebuilding the
candidate:

```sh
cp cmake-build-release/bin/rxbvm BASELINE/rxbvm
cp cmake-build-release/bin/rxtvm BASELINE/rxtvm
cp cmake-build-release/bin/library.rxbin BASELINE/library.rxbin
cp cmake-build-release/tests/performance/decimal_gate1_rexxcps_family_opt.rxbin BASELINE/call_arg.rxbin
cmake --build cmake-build-release --target rxbvm rxtvm --parallel 10
```

Every warmup and recorded cell used the same argument vector, with `VM` and
`VARIANT` selected by the balanced loop:

```sh
VARIANT/VM BASELINE/call_arg.rxbin BASELINE/library.rxbin \
  -a call-arg 5000000
```

Elapsed time was captured around the direct child without changing its output:

```sh
perl -MTime::HiRes=time -e \
  'my $start=time; system @ARGV; my $status=$?; printf STDERR "%.9f\n", time-$start; exit($status == -1 ? 127 : ($status >> 8));' \
  -- VARIANT/VM BASELINE/call_arg.rxbin BASELINE/library.rxbin \
  -a call-arg 5000000
```

For each concrete VM, odd rounds ran baseline then candidate and even rounds
ran candidate then baseline. Medians are the mean of sorted samples six and
seven. Each paired percentage is `(candidate - baseline) / baseline * 100`;
the reported interval is the paired mean plus or minus Student t(11)=2.201
times the sample standard error.
