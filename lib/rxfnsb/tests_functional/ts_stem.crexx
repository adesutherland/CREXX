/* STEM */
options levelb
import rxfnsb

errors=0

s = .stem()

val = s.mykey
if val \= "" then do
   errors=errors+1
   say 'STEM failed in test 1 expected empty string for new key, got: ' val
end

s.mykey = "myvalue"
val = s.mykey
if val \= "myvalue" then do
   errors=errors+1
   say 'STEM failed in test 2 expected myvalue, got: ' val
end

s.key1 = "val1"
s.key2 = "val2"
s.key3 = "val3"

if s.key1 \= "val1" then do
   errors=errors+1
   say 'STEM failed in test 3'
end
if s.key2 \= "val2" then do
   errors=errors+1
   say 'STEM failed in test 4'
end
if s.key3 \= "val3" then do
   errors=errors+1
   say 'STEM failed in test 5'
end

s.key2 = "new_val2"
if s.key2 \= "new_val2" then do
   errors=errors+1
   say 'STEM failed in test 6'
end

s["€uros"] = "100"
s["你好"] = "world"
s["🔥fire"] = "hot"
s.key_with_Ü = "value_Ü"

if s["€uros"] \= "100" then do
  errors=errors+1
  say 'STEM failed in test 7 expected 100 for €uros, got: ' s["€uros"]
end
if s["你好"] \= "world" then do
  errors=errors+1
  say 'STEM failed in test 8 expected world for 你好, got: ' s["你好"]
end
if s["🔥fire"] \= "hot" then do
  errors=errors+1
  say 'STEM failed in test 9 expected hot for 🔥fire, got: ' s["🔥fire"]
end
if s.key_with_Ü \= "value_Ü" then do
  errors=errors+1
  say 'STEM failed in test 10 expected value_Ü for key_with_Ü, got: ' s.key_with_Ü
end

return errors<>0