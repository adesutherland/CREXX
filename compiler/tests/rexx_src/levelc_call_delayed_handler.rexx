options levelc
call seed
call on error name 'handler'
say 'local-after=' || result || '|' || .result || '|' || condition('C')
call on error name ARG
say 'bif-after=' || result || '|' || .result || '|' || condition('C')
call off error
exit
handler:
  say 'caught=' || condition('C') || '|' || condition('D') || '|' || condition('I') || '|' || condition('S') || '|' || sigl || '|' || arg()
  return 'ignored'
seed:
  return 'old'
