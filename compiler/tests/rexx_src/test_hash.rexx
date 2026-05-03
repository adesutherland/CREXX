options levelb
import rxfnsb
h = 0
key = "hello"
len = key.length()
fz = ""
do i = 0 to len - 1
  assembler hexchar fz, key, i
  val = x2d(fz)
  h = h * 31 + val
end
say "hash:" h
q = h % 256
rem = h - q * 256
if rem < 0 then rem = -rem
say "bucket:" (rem + 1)
return 0
