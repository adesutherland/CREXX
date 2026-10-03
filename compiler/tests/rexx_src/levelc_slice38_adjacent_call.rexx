options levelc
x='x'
say 'literal' length('abc')
say x length('ab')
say 'nested' length(substr('abcd',2,2))
say 'empty' length('')
say 'local' echo('Z')
say 'spaced' length ('abc')
exit
echo: procedure
arg value
return value || 'Z'
