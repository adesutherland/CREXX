options levelc
call on halt name caught
nop
say 'after'
exit
caught:
  say 'caught=' || condition('D') || '|' || sigl
  if condition('D') = 'initial' then say 'inside'
  return
