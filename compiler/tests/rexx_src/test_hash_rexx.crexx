options levelb

import rxfnsb

call print_hash("mykey")
call print_hash("hello")
call print_hash("test")
call print_hash("key1")
call print_hash("longstring_to_test_hash_function_123")

return 0

print_hash: procedure
  arg key = .string
  h = 0
  len = 0
  assembler strlen len, key
  if len = 0 then return
  
  fz = ""
  val = 0
  do i = 0 to len - 1
    assembler hexchar fz, key, i
    val = x2d(fz)
    h = h * 31 + val
  end
  
  num_buckets = 256
  q = h % num_buckets
  rem = h - q * num_buckets
  if rem < 0 then rem = -rem
  say "Hash for '" || key || "': h=" || h || ", bucket=" || (rem + 1)
  return
