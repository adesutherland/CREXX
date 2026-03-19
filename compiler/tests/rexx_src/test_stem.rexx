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

/* UTF-8 Tests */
call s.set("€uros", "100")
call s.set("你好", "world")
call s.set("🔥fire", "hot")
call s.set("key_with_Ü", "value_Ü")

if s.get("€uros") \= "100" then do
  say "FAILED: expected 100 for €uros, got: " s.get("€uros")
  return 1
end
if s.get("你好") \= "world" then do
  say "FAILED: expected world for 你好, got: " s.get("你好")
  return 1
end
if s.get("🔥fire") \= "hot" then do
  say "FAILED: expected hot for 🔥fire, got: " s.get("🔥fire")
  return 1
end
if s.get("key_with_Ü") \= "value_Ü" then do
  say "FAILED: expected value_Ü for key_with_Ü, got: " s.get("key_with_Ü")
  return 1
end

say "OK: stem works."
return 0
