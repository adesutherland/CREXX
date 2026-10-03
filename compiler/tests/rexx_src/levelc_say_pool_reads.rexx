options levelc
part='MiX'
other='b'
say 'missing=' || bag.part.other
bag.part='single'
say 'single=' || bag.part
part='MiX.b'
bag.part='multi'
part='MiX'
say 'bound=' || bag.part.other
say 'stem=' || bag.
say 'local=' || local()
call sink bag.part.other
drop bag.
say 'dropped-stem=' || bag.
say 'dropped-compound=' || bag.part.other
exit

local: procedure expose bag. part other
say 'proc=' || bag.part.other
return bag.

sink: procedure
arg item
say 'call=' || item
return
