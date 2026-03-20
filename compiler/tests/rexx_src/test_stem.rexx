options levelb

import rxfnsb

say "Testing stem implementation..."

s = .stem()

val = s.mykey
if val \= "" then do
   say "FAILED: expected empty string for new key, got: " val
   return 1
end

s.mykey = "myvalue"
val = s.mykey
if val \= "myvalue" then do
   say "FAILED: expected myvalue, got: " val
   return 1
end

s.key1 = "val1"
s.key2 = "val2"
s.key3 = "val3"

if s.key1 \= "val1" then return 1
if s.key2 \= "val2" then return 1
if s.key3 \= "val3" then return 1

s.key2 = "new_val2"
if s.key2 \= "new_val2" then return 1

/* UTF-8 Tests */
/* Note: For properties, property names must be valid identifiers.
   Some UTF-8 keys might need bracket notation if they are not valid identifiers,
   but for now we use bracket notation for arbitrary keys */
s["€uros"] = "100"
s["你好"] = "world"
s["🔥fire"] = "hot"
s.key_with_Ü = "value_Ü"

if s["€uros"] \= "100" then do
  say "FAILED: expected 100 for €uros, got: " s["€uros"]
  return 1
end
if s["你好"] \= "world" then do
  say "FAILED: expected world for 你好, got: " s["你好"]
  return 1
end
if s["🔥fire"] \= "hot" then do
  say "FAILED: expected hot for 🔥fire, got: " s["🔥fire"]
  return 1
end
if s.key_with_Ü \= "value_Ü" then do
  say "FAILED: expected value_Ü for key_with_Ü, got: " s.key_with_Ü
  return 1
end

say "OK: stem works."
return 0
