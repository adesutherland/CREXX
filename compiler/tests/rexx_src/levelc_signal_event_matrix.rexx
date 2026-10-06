options levelc
rc = 77
signal on error name missed_error
signal on error name got_error
call raiseevent 2, 'error-É'
exit 90
got_error:
if condition('C') <> 'ERROR' | condition('D') <> 'error-É' then exit 91
if condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' | condition('E') <> '' | rc <> 77 | sigl <> 11 then exit 92
say 'error-ok'
signal on failure name got_failure
call raiseevent 3, 'failure-É'
exit 93
got_failure:
if condition('C') <> 'FAILURE' | condition('D') <> 'failure-É' then exit 94
if condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' | condition('E') <> '' | rc <> 77 | sigl <> 11 then exit 95
say 'failure-ok'
signal on halt
call raiseevent 4, 'halt-É'
exit 96
halt:
if condition('C') <> 'HALT' | condition('D') <> 'halt-É' then exit 97
if condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' | condition('E') <> '' | rc <> 77 | sigl <> 11 then exit 98
say 'halt-ok'
signal on notready name got_notready
call raiseevent 5, 'notready-É'
exit 99
got_notready:
if condition('C') <> 'NOTREADY' | condition('D') <> 'notready-É' then exit 100
if condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' | condition('E') <> '' | rc <> 77 | sigl <> 11 then exit 101
say 'notready-ok'
signal on novalue name got_novalue
call raiseevent 6, 'novalue-É'
exit 102
got_novalue:
if condition('C') <> 'NOVALUE' | condition('D') <> 'novalue-É' then exit 103
if condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' | condition('E') <> '' | rc <> 77 | sigl <> 11 then exit 104
say 'novalue-ok'
signal on lostdigits name got_lostdigits
call raiseevent 7, 'lostdigits-É'
exit 105
got_lostdigits:
if condition('C') <> 'LOSTDIGITS' | condition('D') <> 'lostdigits-É' then exit 106
if condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' | condition('E') <> '' | rc <> 77 | sigl <> 11 then exit 107
say 'lostdigits-ok'
signal on syntax name got_syntax
say substr('abc', 0)
exit 108
got_syntax:
if condition('C') <> 'SYNTAX' | condition('I') <> 'SIGNAL' then exit 109
if condition('S') <> 'OFF' | rc <> 40 | sigl <> 47 then exit 110
say 'syntax-ok'
exit
missed_error:
exit 111
