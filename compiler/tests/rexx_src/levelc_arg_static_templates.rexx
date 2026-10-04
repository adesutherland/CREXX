options levelc
slot='before'
call probe 'ab:cd',,'one two','tail'
say 'exposed=' || slot || '|'
exit

probe:
procedure expose slot
arg left ':' right, , word rest, slot
say 'first=' || left || '|' || right || '|' || word || '|' || rest || '|'
arg again ':' last, , whole, .
say 'repeat=' || again || '|' || last || '|' || whole || '|'
arg , second_again
say 'missing=' || second_again || '|'
arg
return
