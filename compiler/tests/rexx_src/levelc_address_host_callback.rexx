options levelc
address editor 'héllo' with output stem out.
if out.0 <> 1 | out.1 <> 'native:héllo' then exit 20
if rc <> 0 | .rc <> 0 | .rs <> 0 then exit 21
file = 'levelc-address-host-output.txt'
address editor 'write' with output replace stream file
address editor 'append' with output append stream file
error_file = 'levelc-address-host-error.txt'
address editor 'err' with error replace stream error_file
address editor 'errappend' with error append stream error_file
address editor 'errstem' with error stem errors.
if errors.0 <> 1 | errors.1 <> 'err-one' then exit 27
address editor 'errstemappend' with error append stem errors.
if errors.0 <> 2 | errors.2 <> 'err-two' then exit 28
if rc <> 0 | .rc <> 0 | .rs <> 0 then exit 26
call on failure name failed
address editor
'fail'
exit 22
failed:
if condition('C') <> 'FAILURE' | condition('E') <> '' | condition('I') <> 'CALL' | condition('S') <> 'DELAY' then exit 23
if condition('D') <> 'fail' then exit 24
if rc <> -9 | .rc <> -9 | .rs <> -1 then exit 25
if sigl <> 18 then exit 33
call on error name errored
address editor 'error'
if rc <> 7 | .rc <> 7 | .rs <> 1 then exit 29
address editor with output stem implicit.
'héllo🙂'
if implicit.0 <> 1 | implicit.1 <> 'native:héllo🙂' then exit 32
call off failure
call off error
signal on failure name signal_failed
address editor 'signalfail'
exit 34
signal_failed:
if condition('C') <> 'FAILURE' | condition('D') <> 'signalfail' then exit 35
if condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' | condition('E') <> '' then exit 36
if rc <> -9 | .rc <> -9 | .rs <> -1 then exit 37
if sigl <> 34 then exit 42
signal on error name signal_errored
address editor 'signalerror'
exit 38
signal_errored:
if condition('C') <> 'ERROR' | condition('D') <> 'signalerror' then exit 39
if condition('I') <> 'SIGNAL' | condition('S') <> 'OFF' | condition('E') <> '' then exit 40
if rc <> 7 | .rc <> 7 | .rs <> 1 then exit 41
if sigl <> 42 then exit 43
exit 0
errored:
if condition('C') <> 'ERROR' | condition('D') <> 'error' | condition('E') <> '' | condition('I') <> 'CALL' | condition('S') <> 'DELAY' then exit 30
if sigl <> 26 then exit 31
return
