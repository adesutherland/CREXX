options levelc
value = 'outer'
signal on syntax name caught
call worker
say 'after'
exit
worker:
procedure expose value
value = 'inner'
say substr('abc',0)
return
caught:
say value || '|' || rc || '|' || sigl
return
