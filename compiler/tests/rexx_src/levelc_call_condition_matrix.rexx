options levelc
call seed
call on error name caught
nop
call off error
call on failure name caught
nop
call off failure
call on halt name caught
nop
call off halt
call on notready name caught
nop
call off notready
say 'after=' || result || '|' || .result || '|' || condition('C')
exit
caught:
  say 'caught=' || condition('C') || '|' || condition('D') || '|' || condition('I') || '|' || condition('S') || '|' || sigl || '|' || arg()
  return 'ignored'
seed:
  return 'prior'
