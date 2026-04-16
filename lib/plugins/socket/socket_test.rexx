options levelb
import rxfnsb
import socket

say "socket smoke"

s = socketCreate()
if s < 0 then do
    say "socketCreate failed" s
    exit 1
end

err = socketLastError(s)
if err \= "0 " then do
    say "unexpected initial socket error" err
    call socketClose s
    exit 1
end

rc = socketClose(s)
if rc \= 0 then do
    say "socketClose failed" rc
    exit 1
end

say "socket smoke ok"
return 0
