options levelc
original='aBc'
other='dEf'
say 'result' probe(original,other)
say 'after' original other
say 'spaces' probe('a b','c d')
say 'empty' length(echo(''))
exit
probe: procedure expose original
arg first, second
say 'inside' first second original
return first || '/' || second
echo: procedure
arg value
return value
