options levelc
call seed
call on error
say 'first'
say 'second'
call off error
say 'off'
call on error
call child
say 'after-child'
say 'parent=' || condition('C') || '|' || result
exit
error:
  say 'caught=' || condition('C') || '|' || condition('D') || '|' || condition('S') || '|' || condition('I') || '|' || arg() || '|' || result || '|' || sigl
  return 'ignored'
seed:
  return 'old'
child:
  call off error
  return 'old'
