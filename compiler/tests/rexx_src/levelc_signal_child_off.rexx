options levelc
signal on syntax name caught
call worker
say substr('abc',0)
exit
worker:
signal off syntax
return
caught:
say rc || '|' || sigl
return
