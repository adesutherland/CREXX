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
plain='410042'x
say 'unicode-scalar=' || c2x(plain)
first='é'
second='🙂'
items.first.second='漢🙂'
say 'unicode-tail=' || items.first.second
first='FF0080'x
items.first.second='🌍'
say 'nul-tail=' || items.first.second
first='old'
second='tail'
items.first.second='old-value'
items.first.second=unicodechange()
old='old'
tail='tail'
say 'unicode-order=' || items.first.second || '|' || items.old.tail
say 'unicode-local=' || unicodelocal()
say 'unicode-exposed=' || items.first.second
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

unicodechange: procedure expose first second
first='é'
second='🙂'
return '漢'

unicodelocal: procedure expose items. first second
items.first.second='🙂'
return items.first.second
