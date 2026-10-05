options levelc
word = 'main'
call outer 'a b',,'c d'
say 'main=' || word
exit
outer:
arg first, hole, third
say 'outer=' || first || '|' || hole || '|' || third || '|' || arg(2,'E')
call inner 'x',,'z'
next:
arg repeat
say 'repeat=' || repeat || '|' || arg(2,'E')
return
inner:
procedure expose word
arg value, gap, tail
word = value
say 'inner=' || value || '|' || gap || '|' || tail
return
