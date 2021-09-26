/* Level B Function Test */
options levelb
x = 2.0
a = 1
say "result =" test( a,"sss",x)
say "main() a =" a
say "main() x =" x
  say ""

say "result =" test(a,,x,10)
say "main() a =" a
say "main() x =" x
  say ""


/*
say "main() calling test 1, 2.0, 5.0"
call test a,"sss",x
say "main() a =" a "(should be 1)"
say "main() x =" x "(should be 10)"
*/
/* say "Length of 'hello' is" length("hello") */
return

test: procedure = .int
  arg a = .int, message = "empty", expose b = .float,  c = b + 1
  result = 0.0
  result = a + b + double(c)
  say "test() message =" message
  say "test() a =" a
  say "test() b =" b
  say "test() c =" c
  a = -1
  message = "changed"
  b = -2
  c = -3

  return result

double: procedure = .float
  arg x = .float
  result = x * 2.0
/*  say "double() double" x "is" result */
  return result

/*
length: procedure = .int
  arg string1 = .string
*/