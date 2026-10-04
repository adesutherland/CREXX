options levelc
sep=':'
pos=3
parse value 'mn:op' with parse_left (sep) parse_right
say 'parse=' || parse_left || '|' || parse_right || '|'
call probe 'ab:cd', 'abcdef', 'x:b:tail'
exit

probe:
procedure expose sep pos
arg left (sep) right, first =(pos) second, prefix ':' marker (marker) rest
say 'literal=' || left || '|' || right || '|'
say 'position=' || first || '|' || second || '|'
say 'capture=' || prefix || '|' || marker || '|' || rest || '|'
return
