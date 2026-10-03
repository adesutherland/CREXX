options levelc
item='a'
say 'literal' '['item']'
say 'symbol' item'!'
say 'group-left' ('a')'b'
say 'call-left' length('abc')'x'
say 'call-right' 'x'length('a')
say 'spaced' '[' item ']'
say 'comment' '['/*gap*/item']'
say 'comment-spaced' '[' /*gap*/ item ']'
say 'explicit' '[' || item || ']'
parse value '['item']' with parsed
say 'parse' parsed
parse value '[' item ']' with parsed
say 'parse-spaced' parsed
