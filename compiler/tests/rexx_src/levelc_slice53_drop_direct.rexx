options levelc
items.a='one'
items.b='two'
key='b'
items.key='lower'
say 'before=' || items.a || '|' || items.b || '|' || items.key || '|'
drop items.a
say 'tail=' || items.a || '|' || items.b || '|'
drop items.key
say 'derived=' || items.key || '|' || items.b || '|'
items.a='again'
say 'restore=' || items.a || '|'
name='a'
items.name='lower-a'
drop name items.name
say 'order=' || name || '|' || items.name || '|' || items.a || '|'
drop items.
say 'stem=' || items.a || '|' || items.b || '|' || items.key || '|'
items.1='digit'
drop items.1
say 'numeric=' || items.1 || '|'
items.a='parent'
say 'local=' || local() || '|' || items.a || '|'
if 1 then drop items.b
say 'if=' || items.b || '|'
do
  items.b='block'
  drop items.b
end
say 'do=' || items.b || '|'
exit
local: procedure expose items.
drop items.a
items.b='inside'
return items.a || '/' || items.b
