/* UTF-8 Characters and Math Operations Tests */
options levelb
import rxfnsb

say "Testing UTF-8 and Math..."

s = "🔥fire"
l = length(s)
say "Length of string is" l

c1 = substr(s, 1, 1)
v = c2d(c1)
say "c2d of first char is" v

/* Math with large numbers */
large_num = v * 1000000
rem = large_num % 256
say "large_num % 256 =" rem

div = large_num / 256
say "large_num / 256 =" div

return 0
