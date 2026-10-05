options levelc
say 'empty=[' || empty() || ']'
say 'fall=' || f('x')
say 'recursive=' || total(3)
call sub
say 'done'
exit
empty:
return ''
f:
arg value
next:
return value || 'y'
total:
procedure
arg n
if n < 1 then return ''
return n || total(n - 1)
sub:
return
