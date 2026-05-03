options levelb
import rxfnsb
h = 0
key = "mykey"
len = key.length()
do i = 1 to len
  char = key.substr(i, 1)
  val = c2d(char)
  h = h * 31 + val
end
num_buckets = 256
q = h % num_buckets
rem = h - q * num_buckets
if rem < 0 then rem = -rem
say "Hash for '" || key || "': h=" || h || ", bucket=" || (rem + 1)
return 0
