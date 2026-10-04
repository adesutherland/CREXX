options levelc
a='one'
b='two'
names='a b'
drop (names)
say 'pair=' || a || '|' || b || '|' || names || '|'
a='again'
b='back'
names='a / 1bad b'
drop (names)
say 'invalid=' || a || '|' || b || '|'
key='b'
items.key='lower'
items.B='upper'
names='items.key items.B'
drop (names)
say 'compound=' || items.key || '|' || items.B || '|'
items.key='lower2'
items.B='upper2'
names='key items.key'
drop (names)
say 'order=' || key || '|' || items.key || '|'
key='b'
say 'retained=' || items.key || '|' || items.B || '|'
names='items.'
drop (names)
say 'stem=' || items.key || '|' || items.B || '|'
a='held'
names='a'
drop names (names)
say 'outer=' || names || '|' || a || '|'
key='x'
lists.key='a'
drop (lists.key)
say 'reference=' || a || '|'
items.B='parent'
names='items.B'
say 'local=' || local() || '|' || items.B || '|'
names='a'
a='if-value'
if 1 then drop (names)
say 'if=' || a || '|'
do
  a='do-value'
  drop (names)
end
say 'do=' || a || '|'
a='unicode-a'
b='unicode-b'
names='a' || 'A0'x || '🙂' || 'A0'x || 'b'
drop (names)
say 'unicode=' || a || '|' || b || '|'
exit
local: procedure expose items. names
drop (names)
return items.B
