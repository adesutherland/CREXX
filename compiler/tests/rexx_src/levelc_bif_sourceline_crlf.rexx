options levelc
/* retained comment */

say sourceline()
say sourceline('+0002')
say length(sourceline(3))
call nested
signal on syntax name bad
say sourceline(999999999999999999999999999)
exit
nested: procedure
  say sourceline(1)
  return
bad:
  say rc sigl condition('c')
  say pos('17',condition('d')) > 0
  exit