options levelb

import rxfnsb

say "Testing stem implementation..."

s = .stem()

val = s.get("mykey")
if val \= "" then do
   say "FAILED: expected empty string for new key, got: " val
   return 1
end

call s.set("mykey", "myvalue")
val = s.get("mykey")
if val \= "myvalue" then do
   say "FAILED: expected myvalue, got: " val
   return 1
end

call s.set("key1", "val1")
call s.set("key2", "val2")
call s.set("key3", "val3")

if s.get("key1") \= "val1" then return 1
if s.get("key2") \= "val2" then return 1
if s.get("key3") \= "val3" then return 1

call s.set("key2", "new_val2")
if s.get("key2") \= "new_val2" then return 1

say "OK: stem works."
return 0
