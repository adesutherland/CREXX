options levelc
s='  a  b   c  d e  '
parse var s first second third fourth tail
say 'five=' || first || '|' || second || '|' || third || '|' || fourth || '|' || tail || '|'
parse var s . second . fourth .
say 'dots=' || second || '|' || fourth || '|'
parse var s . . . . .
say 'alldots=' || s || '|'
s='a b'
parse var s first second third fourth fifth sixth seventh tail
say 'short=' || first || '|' || second || '|' || third || '|' || fourth || '|' || fifth || '|' || sixth || '|' || seventh || '|' || tail || '|'
s='left one two three four five six'
parse var s s second third fourth fifth tail
say 'alias=' || s || '|' || second || '|' || third || '|' || fourth || '|' || fifth || '|' || tail || '|'
parse value 'w x y z q r' with same same same same same same
say 'repeat=' || same || '|'
hits=0
parse value nextvalue() with first second third fourth fifth tail
say 'value=' || first || '|' || second || '|' || third || '|' || fourth || '|' || fifth || '|' || tail || '|' || hits
parse upper value ' a b c d e f g ' with first . third . fifth tail
say 'upper=' || first || '|' || third || '|' || fifth || '|' || tail || '|'
say 'local=' || local() || '|'
exit
nextvalue: procedure expose hits
hits=hits+1
return 'm n o p q r s'
local: procedure
parse value ' a  b   c d e f ' with first second third fourth fifth tail
return first || '|' || second || '|' || third || '|' || fourth || '|' || fifth || '|' || tail
