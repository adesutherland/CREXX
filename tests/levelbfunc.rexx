/* Level B Function Test */
options levelb
x = 2.0
a = 1
say "main() calling test 1, 2.0, 5.0"
call test a, x, x+3.0
say "main() a =" a "(should be 1)"
say "main() x =" x "(should be 10)"
say "Length of 'hello' is" length("hello")
return

test: procedure = .int
  arg a = .int, b .float, c = .float
  result = 0.0
  result = a + b + double(c)
  say "test() a =" a
  say "test() b =" b
  say "test() c =" c
  say "test() a + b + c*2 =" result
  say "test() setting a and b (aka x) to 10"
  a = 10
  b = 10
  return result

double: procedure = .float
  arg x = .float
  result = x * 2.0
  say "double() double" x "is" result
  return result

length: procedure = .int
  arg string1 = .string
