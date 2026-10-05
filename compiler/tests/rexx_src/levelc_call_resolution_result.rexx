options levelc
rc = 77
result = 'prior'
call setter 'A',, ''
say 'local=' || result || '|' || .result || '|' || symbol('RESULT') || '|' || rc
call voider
say 'void=' || symbol('RESULT') || '|' || symbol('.RESULT') || '|' || result
call empty_result
say 'empty=[' || result || ']|' || symbol('RESULT')
call length 'abc'
say 'shadow=' || result
call 'LENGTH' 'abc'
say 'quoted=' || result
call '4C 45 4E 47 54 48'x 'abcd'
say 'hex-target=' || result
call '01001100 01000101 01001110 01000111 01010100 01001000'b 'abcde'
say 'binary-target=' || result
say 'grouped=' || c2x('0100 1100 01000101'b) || '|' || c2x('1 0000'b) || '|' || c2x('00001 0000'b)
parse value '01001100 01000101'b with parsed_group
say 'parsed=' || parsed_group
call arg
say 'main-argc=' || result
call outer
say 'outer=' || result || '|' || .result
call 7
call 1.2
call .5
call 7E2
call 7dogs
call .foo
call ..foo
call .5abc
call .
call .RESULT
call on error name .RESULT
call off error
call loose
say 'fallthrough=' || symbol('RESULT') || '|' || result
exit

setter:
  arg first, second, third
  say 'args=' || arg() || '|' || arg(1,'E') || '|' || arg(2,'E') || '|' || arg(3,'E') || '|' || first || '|' || second || '|' || third
  return 'done'

voider:
  return

empty_result:
  return ''

length:
  return 'local-length'

outer:
  call inner
  say 'inner=' || result
  return 'outer-result'

inner:
  return 'inner-result'

7:
  say 'integer'
  return

1.2:
  say 'decimal'
  return

.5:
  say 'leading-dot'
  return

7E2:
  say 'exponent'
  return

7dogs:
  say 'constant'
  return

.foo:
  say 'dot-letter'
  return

..foo:
  say 'two-dots'
  return

.5abc:
  say 'dot-digit-symbol'
  return

.:
  say 'single-dot'
  return

.RESULT:
  say 'reserved-dot'
  return

loose:
  say 'loose'
