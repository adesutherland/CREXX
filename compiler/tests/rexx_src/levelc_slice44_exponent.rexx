options levelc
say 'literal' 1e2 1e+2 1e-2 1.5e2 1.e2 .5e+1
say 'math' 1e2+1 1e-2+1
parse value 1e2 with parsed
say 'parse' parsed
say 'abutted' '['1e2']'
do count=1 for 1e0
  say 'whole' count
end count
