/* This is a simple test */
options levelb
say a b
say a || b
say a "b"
say a"b"

if a=1 then do
  b = 1
  c=3
end
else
 do
  a = b = 1
  c = 4
end
d = 2 * (b + c)
"listfile"
say d
