options levelc
s='  a  b   c d  '
parse var s first .
say 'final2=' || first || '|' || s || '|'
parse var s . rest
say 'initial2=' || rest || '|'
parse var s first . tail
say 'middle3=' || first || '|' || tail || '|'
parse var s . second tail
say 'initial3=' || second || '|' || tail || '|'
parse var s . . tail
say 'double3=' || tail || '|'
parse var s first second .
say 'final3=' || first || '|' || second || '|'
parse var s first . .
say 'doublefinal3=' || first || '|'
s='a'
parse var s . second tail
say 'short=' || second || '|' || tail || '|'
s='left right tail more'
parse var s s . tail
say 'alias=' || s || '|' || tail || '|'
parse value 'x y z q' with same . same
say 'repeat=' || same || '|'
hits=0
parse value nextvalue() with . second tail
say 'value=' || second || '|' || tail || '|' || hits
parse upper value ' a   b   c d ' with . second tail
say 'upper=' || second || '|' || tail || '|'
say 'local=' || local() || '|'
exit
nextvalue: procedure expose hits
hits=hits+1
return 'm n o p'
local: procedure
parse value ' a  b   c d ' with first . tail
return first || '|' || tail
