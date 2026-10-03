options levelc
s='  a  b   c  '
parse var s first second tail
say first || '|' || second || '|' || tail || '|' || s
s='a'
parse var s first second tail
say first || '|' || second || '|' || tail
s='  a  b   c  '
parse var s same same same
say same || '|'
s='left right tail here'
parse var s s second tail
say s || '|' || second || '|' || tail
hits=0
parse value nextvalue() with first second tail
say first || '|' || second || '|' || tail || '|' || hits
parse upper value ' a   b   c ' with first second tail
say first || '|' || second || '|' || tail || '|'
say 'local' local()
exit
nextvalue: procedure expose hits
hits=hits+1
return 'x y z  q'
local: procedure
parse value ' a  b   c ' with first second tail
return first || '|' || second || '|' || tail
