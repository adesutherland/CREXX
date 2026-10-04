options levelc
call probe 'aé🙂b', 'FF'x
say 'function=' || echo('é🙂')
exit

probe: procedure
arg first 3 rest, high
say 'fields=' || first || '|' || rest || '|' || high
say 'raw=' || arg(1) || '|' || c2x(arg(2))
arg repeat
say 'repeat=' || repeat
if 1 then call dot 'é🙂'
return

dot: procedure
arg . 2 tail
say 'dot=' || tail
return

echo: procedure
arg value
return value
