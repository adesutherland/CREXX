options levelc
signal on novalue name outer
call worker
say missing
exit
worker:
procedure
signal off novalue
say unbound
signal on novalue name inner
say absent
say 'bad'
return
inner:
say 'inner=' || sigl || '|' || rc
return
outer:
say 'outer=' || sigl || '|' || rc
