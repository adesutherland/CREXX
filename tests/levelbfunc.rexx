/* Level B Function Test */
options levelb

a = 1; b = 2

if test1(, a, b) <> 3 then say ERROR
if a <> 1 then say ERROR
if b <> 2 then say ERROR

s = "test2"; a = 1; b = 2
if test2(s, a, b) <> 3 then say ERROR
if a <> 1 then say ERROR
if b <> 2 then say ERROR

s = "test3"; a = 1
if test3(s, a) <> 3 then say ERROR
if a <> 1 then say ERROR

s = "test4"; a = 1; b = 2
if test4(s, a, b) <> 3 then say ERROR
if a <> 10 then say ERROR
if b <> 20 then say ERROR

s = "test5"; af = 1.0; b = 2
if test3(s, af, b) <> 3 then say ERROR
if af <> 1 then say ERROR
if b <> 2 then say ERROR

/* Syntax error
s = "test6"; af = 1.0; b = 2
if test4(s, af, b) <> 3 then say ERROR
if af <> 1 then say ERROR
if b <> 2 then say ERROR
*/

a = 1; b = 2
if test1("test7", a, b) <> 3 then say ERROR
if a <> 1 then say ERROR
if b <> 2 then say ERROR

a = 1; b = 2
if test5("test8", a, b) <> 3 then say ERROR
if a <> 10 then say ERROR
if b <> 20 then say ERROR

s = "test9"; a = 1; b = 2
if test1(s, a, b) <> 3 then say ERROR
if a <> 1 then say ERROR
if b <> 2 then say ERROR

a = 1; b = 2
if test6("test10", a, b) <> 3 then say ERROR
if a <> 10 then say ERROR
if b <> 20 then say ERROR

s = "test11"; a = 1; b = 2
if test6(s, a, b) <> 3 then say ERROR
if a <> 10 then say ERROR
if b <> 20 then say ERROR


test1: procedure = .int
  arg message = "test1", x = .int, y = .int
  say message
  message = ""
  r = x + y
  x = 10; y=20;
return r

test2: procedure = .int
  arg message = .string, x = .int, y = .int
  say message
  r = x + y
  x = 10; y=20;
return r

test3: procedure = .int
  arg message = .string, x = .int, y = 2
  say message
  r = x + y
  x = 10; y=20;
return r

test4: procedure = .int
  arg message = .string, expose x = .int, expose y = 2
  say message
  r = x + y
  x = 10; y=20;
return r

test5: procedure = .int
  arg message = "test1", expose x = .int, expose y = .int
  say message
  message = ""
  r = x + y
  x = 10; y=20;
return r

test6: procedure = .int
  arg message = "test1", expose x = .int, expose y = .int
  say message
  r = x + y
  x = 10; y=20;
return r



/*
say "main() calling test 1, 2.0, 5.0"
call test a,"sss",x
say "main() a =" a "(should be 1)"
say "main() x =" x "(should be 10)"
*/
/* say "Length of 'hello' is" length("hello") */

/*return */
/*
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
  say "double() double" x "is" result
  return result
*/
/*
length: procedure = .int
  arg string1 = .string
*/