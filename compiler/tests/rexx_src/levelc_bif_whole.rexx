options levelc
numeric digits 2
numeric fuzz 1
say substr('abc','1.0')
say substr('abc','1e0')
say c2x(left('ab','3.0'))
say copies('x','2e0')
say word('a b','2.00')
say sourceline('1.0')
call nested 'actual'
signal on syntax name bad
say substr('abc','1.01')
exit
nested: procedure
  say arg('1.0')
  return
bad:
  say rc sigl condition('c')
  exit
