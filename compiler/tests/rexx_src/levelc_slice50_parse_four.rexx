options levelc
s='  a  b   c  d e  '
parse var s first second third tail
say 'four=' || first || '|' || second || '|' || third || '|' || tail || '|'
parse var s first . third tail
say 'internal=' || first || '|' || third || '|' || tail || '|'
parse var s . second . tail
say 'multiple=' || second || '|' || tail || '|'
parse var s . . . tail
say 'final=' || tail || '|'
parse var s first . third .
say 'finaldrop=' || first || '|' || third || '|'
parse var s . . . .
say 'alldrop=' || s || '|'
s='a b c'
parse var s first second third tail
say 'short=' || first || '|' || second || '|' || third || '|' || tail || '|'
s='left right third tail more'
parse var s s second third tail
say 'alias=' || s || '|' || second || '|' || third || '|' || tail || '|'
parse value 'w x y z q' with same same same same
say 'repeat=' || same || '|'
hits=0
parse value nextvalue() with first second third tail
say 'value=' || first || '|' || second || '|' || third || '|' || tail || '|' || hits
parse upper value ' a b c d e ' with first . third tail
say 'upper=' || first || '|' || third || '|' || tail || '|'
say 'local=' || local() || '|'
exit
nextvalue: procedure expose hits
hits=hits+1
return 'm n o p q'
local: procedure
parse value ' a  b   c d e ' with first second third tail
return first || '|' || second || '|' || third || '|' || tail
