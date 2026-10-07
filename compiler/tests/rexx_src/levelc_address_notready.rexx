options levelc
signal on notready name missing
f = 'missing-levelc-address-input-20261006'
address system with input stream f
'cat'
exit 90
missing:
if condition('E') <> '' | condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' then exit 93; say 'notready=' || condition('C') || '/' || condition('D') || '/' || sigl
signal off notready
address system 'cat' with input stream f
say 'after=' || rc || '/' || .rc || '/' || .rs
signal on notready name output_missing
badpath = 'missing-address-directory-20261006/out'
address crexx 'echo hi' with output stream badpath
exit 91
output_missing:
if condition('E') <> '' | condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' then exit 94; say 'output=' || condition('C') || '/' || condition('D') || '/' || sigl
signal on syntax name bad_stem
address crexx with input stem missing.
'echo should-not-run'
exit 92
bad_stem:
say 'stem=' || condition('C') || '/' || condition('D') || '/' || sigl
