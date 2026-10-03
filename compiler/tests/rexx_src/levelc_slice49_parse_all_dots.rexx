options levelc
s='  a  b c '
hits=0
parse var s .
say 'one=' || s || '|' || hits
parse var s . .
say 'two=' || s || '|' || hits
parse var s . . .
say 'three=' || s || '|' || hits
parse value nextvalue() with .
say 'value1=' || hits
parse value nextvalue() with . .
say 'value2=' || hits
parse upper value nextvalue() with . . .
say 'value3=' || hits
say 'local=' || local() || '|' || hits
exit
nextvalue: procedure expose hits
hits=hits+1
return ' x y z '
local: procedure expose hits
parse value nextvalue() with .
return 'done'
