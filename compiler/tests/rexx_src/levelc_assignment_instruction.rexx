options levelc
plain='first'
plain='second'
say 'scalar=' || plain
if='keyword'
say 'keyword=' || if
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
blank='x'
blank=
say 'empty-scalar=' || length(blank)
items.key.part='x'
items.key.part=
say 'empty-compound=' || length(items.lookup)
items.='x'
items.=
say 'empty-stem=' || length(items.) || '|' || length(items.lookup)
if 1 then blank='x'
if 1 then blank=
say 'empty-nested=' || length(blank)
items.key.part='x'
call clear
say 'empty-local=' || length(items.lookup)
exit

change: procedure expose key part
key='green'
part='yellow'
return 'captured'

local: procedure expose items. key part
items.key.part='local'
return

clear: procedure expose items. key part
items.key.part=
return
