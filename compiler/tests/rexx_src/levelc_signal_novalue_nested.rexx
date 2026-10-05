options levelc
signal on novalue name caught
call worker
say 'after'
exit
worker:
procedure expose shared
say missing
return
caught:
say 'inner=' || sigl || '|' || rc
return
