options levelc
s='  a  b   c d e  '
parse var s first second third .
say first || '|' || second || '|' || third || '|' || s
s='a b'
parse var s first second third .
say first || '|' || second || '|' || third
s='x y z q'
parse var s same same same .
say same || '|'
s='left right third tail'
parse var s s second third .
say s || '|' || second || '|' || third
hits=0
parse value nextvalue() with first second third .
say first || '|' || second || '|' || third || '|' || hits
parse upper value ' a   b   c d ' with first second third .
say first || '|' || second || '|' || third || '|'
say 'local' local()
exit
nextvalue: procedure expose hits
hits=hits+1
return 'm n o p'
local: procedure
parse value ' a  b   c d ' with first second third .
return first || '|' || second || '|' || third
