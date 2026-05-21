/* rxpa native payload copy/finalizer test */

options levelb
import rxpatests

say "RXPA Native Payload Tests"

call native_payload_reset

payload = native_payload_make(77)
copy = payload

if native_payload_id(payload) \= 77 then do
    say 'native_payload_make() produced unexpected payload id' native_payload_id(payload)
    return 1
end

if native_payload_id(copy) \= 77 then do
    say 'native payload copy produced unexpected payload id' native_payload_id(copy)
    return 1
end

if native_payload_copies() < 1 then do
    say 'native payload copy hook was not called'
    return 1
end

stale = native_payload_make(88)
finalizers_before = native_payload_finalizers()
stale = 0
if native_payload_finalizers() <= finalizers_before then do
    say 'native payload finalizer was not called when scalar overwrite reused a register'
    return 1
end

call native_payload_clear payload
if native_payload_finalizers() < 1 then do
    say 'native payload finalizer was not called for original payload'
    return 1
end

call native_payload_clear copy
if native_payload_finalizers() < 2 then do
    say 'native payload finalizer was not called for copied payload'
    return 1
end

say "OK"

return 0
