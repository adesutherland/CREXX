options levelc
signal on notready name missing
f = 'missing-levelc-address-input-20261006'
address system 'cat' with input stream f
exit 90
missing:
say 'notready=' || condition('C') || '/' || condition('D') || '/' || sigl
signal off notready
address system 'cat' with input stream f
say 'after=' || rc || '/' || .rc || '/' || .rs
signal on notready name output_missing
badpath = 'missing-address-directory-20261006/out'
address crexx 'echo hi' with output stream badpath
exit 91
output_missing:
say 'output=' || condition('C') || '/' || condition('D') || '/' || sigl
signal on syntax name bad_stem
address crexx 'echo should-not-run' with input stem missing.
exit 92
bad_stem:
say 'stem=' || condition('C') || '/' || condition('D') || '/' || sigl
