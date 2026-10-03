options levelc
s='ab,cd,ef'
parse var s a ',' b ',' c
say 'patterns=' || a || '|' || b || '|' || c || '|'
parse var s . ',' middle ',' .
say 'dots=' || middle || '|' || s || '|'
parse var s . ',' .
say 'alldots=' || s || '|'
parse value ',,end,' with leading ',' empty ',' rest ',' trailing
say 'edges=' || leading || '|' || empty || '|' || rest || '|' || trailing || '|'
parse var s first 'Z' last
say 'missing=' || first || '|' || last || '|'
parse var s s ',' part
say 'alias=' || s || '|' || part || '|'
parse value 'w,x,y,z,q,r' with one ',' two ',' three ',' four ',' five ',' six
say 'long=' || one || '|' || two || '|' || three || '|' || four || '|' || five || '|' || six || '|'
s='abcdef'
parse var s a 3 b
say 'absolute=' || a || '|' || b || '|'
parse var s a =4 b +2 c
say 'relative=' || a || '|' || b || '|' || c || '|'
parse var s 3 a +2 b -1 c
say 'backward=' || a || '|' || b || '|' || c || '|'
parse var s a 0 b
say 'zero=' || a || '|' || b || '|'
parse var s a 100 b
say 'far=' || a || '|' || b || '|'
parse var s a 3 b 'd' =03 c
say 'spelling=' || a || '|' || b || '|' || c || '|'
s='a,b'
parse var s a '2c'x b
say 'hex=' || a || '|' || b || '|'
parse var s a '00101100'b b
say 'binary=' || a || '|' || b || '|'
s="a'b,c"
parse var s a "a'b" b
say 'quote=' || a || '|' || b || '|'
s="a'b,c"
parse var s a 'a''b' b
say 'doublequote=' || a || '|' || b || '|'
s='aébc'
parse var s a 'é' b
say 'utf8=' || a || '|' || b || '|'
s='ab,cd,ef'
parse var s ',' a
say 'leading=' || a || '|'
parse var s a ','
say 'trailing=' || a || '|'
parse var s a ',' ',' b
say 'adjacent=' || a || '|' || b || '|'
parse var s 3 a
say 'startabs=' || a || '|'
parse var s a 3 5 b
say 'adjpos=' || a || '|' || b || '|'
parse var s -1 a
say 'startrel=' || a || '|'
hits=0
parse value nextvalue() with a ',' b
say 'once=' || a || '|' || b || '|' || hits || '|'
parse upper value 'ab,cd' with a ',' b
say 'upper=' || a || '|' || b || '|'
say 'local=' || local() || '|'
exit
nextvalue: procedure expose hits
hits=hits+1
return 'm,n'
local: procedure
parse value 'left,right' with a ',' b
return a || '|' || b
