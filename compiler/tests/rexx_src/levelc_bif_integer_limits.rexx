options levelc
say length(arg(9223372036854775807))
say length(word('a',9223372036854775807))
say lastpos('a','abc',9223372036854775807)
say abbrev('abc','a',9223372036854775807)
say length(copies('',9223372036854775807))
signal on syntax name beyond
say arg(9223372036854775808)
exit
beyond:
  say rc sigl (pos('40.12',condition('d')) > 0)
  signal on syntax name exponent
  say word('abc',1e999999999)
  exit
exponent:
  say rc sigl (pos('40.12',condition('d')) > 0)
  exit
