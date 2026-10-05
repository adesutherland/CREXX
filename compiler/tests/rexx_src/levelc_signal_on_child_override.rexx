options levelc
signal on syntax name outer
call child
say 'parent'
say substr('abc',0)
say 'skip'
exit
child:
signal on syntax name inner
say substr('abc',0)
return
inner:
say 'inner=' || rc || '|' || sigl
return
outer:
say 'outer=' || rc || '|' || sigl
return
