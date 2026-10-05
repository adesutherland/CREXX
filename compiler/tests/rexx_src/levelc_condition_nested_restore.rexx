options levelc
signal on novalue name outer
say missing
exit
outer:
say 'outer=' || condition('C') || '|' || condition('D')
call child
say 'after=' || condition('C') || '|' || condition('D') || '|' || condition('S')
return
child:
procedure
signal on syntax name inner
say substr('abc', 0)
return
inner:
say 'inner=' || condition('C') || '|' || condition('E')
return
