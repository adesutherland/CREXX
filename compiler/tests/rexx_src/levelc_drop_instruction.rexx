options levelc
a='A'
b='B'
c='C'
d='D'
e='E'
drop a b c d e
say 'list=' || a || '|' || b || '|' || c || '|' || d || '|' || e || '|'
x='part'
y='Tail'
z='last'
key='part.Tail.last'
items.key='first'
say 'before=' || items.x.y.z
drop items.x.y.z
say 'direct=' || items.key
items.key='second'
names='items.x.y.z'
drop (names)
say 'indirect=' || items.key
items.key='third'
drop x items.x.y.z
say 'order=' || items.key || '|' || items.x.y.z
x='part'
names='items.x.y.z'
say 'local=' || local() || '|' || items.key
items.key='final'
drop items.
say 'stem=' || items.key
exit

local: procedure expose items. x y z key names
drop (names)
return items.key
