options levelc
say queued()
queue 'tail'
push '頭' || '00'x
say queued() queued()
parse pull front
say length(front) c2x(substr(front,2))
say queued()
parse pull tail
say tail queued()
call nested
say queued()
signal on syntax name bad
say queued('extra')
say 'unreachable'
exit
nested: procedure
  queue 'nested'
  say queued()
  return
bad:
  say rc sigl condition('c')
  exit
