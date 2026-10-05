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
call arg
say 'main-argc=' || result
call outer
say 'outer=' || result || '|' || .result
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
