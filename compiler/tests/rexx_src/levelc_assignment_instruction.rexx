options levelc
plain='first'
plain='second'
say 'scalar=' || plain
bytes='410042'x
say 'bytes=' || length(bytes)
key='red'
part='blue'
lookup='red.blue'
newlookup='green.yellow'
items.key.part='first'
say 'compound=' || items.lookup
items.='default'
say 'reset=' || items.lookup
items.key.part=change()
say 'captured=' || items.lookup || '|' || items.newlookup
items.key.part='new'
say 'changed=' || items.newlookup
key='red'
part='blue'
items.='again'
if 1 then items.key.part='if'
do 1
  items.key.part='do'
end
say 'nested=' || items.lookup
call local
say 'exposed=' || items.lookup
say 'stem=' || items.
exit

change: procedure expose key part
key='green'
part='yellow'
return 'captured'

local: procedure expose items. key part
items.key.part='local'
return
