options levelc
s='  a  b   c  '
parse var s first rest
say first || '|' || rest || '|' || s
s='a'
parse var s first rest
say first || '|' || rest
s='  a  b   c  '
parse var s same same
say same || '|'
s='left right tail here'
parse var s s rest
say s || '|' || rest
hits=0
parse value nextvalue() with first rest
say first || '|' || rest || '|' || hits
parse upper value ' a   b   c ' with first rest
say first || '|' || rest || '|'
say 'local' local()
exit
nextvalue: procedure expose hits
hits=hits+1
return 'x y z  q'
local: procedure
parse value ' a  b   c ' with first rest
return first || '|' || rest
