options levelc
x='changed'
e='changed'
say 'literal' 1x 1e 1.2.3 1a.b 123abc 1e2x
say 'pool' 1x 1e
say 'abutted' '['1x']'
parse value 1a.b with parsed
say 'parse' parsed
