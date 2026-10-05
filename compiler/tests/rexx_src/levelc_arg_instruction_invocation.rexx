options levelc
call start 'a b',, ''
call adjacency 'x:y'
say 'main=' || arg()
exit
start:
arg first second,,empty
say 'initial=' || first || '|' || second || '|' || empty || '|' || arg(2,'E') || '|' || arg(3,'E')
if 1 then arg first_only
select
  when 1 then arg again
end
signal joined
joined:
arg repeated
say 'repeat=' || first_only || '|' || again || '|' || repeated
call noargs
say 'function=' || fun('mixed')
return
adjacency:
sep = ':'
arg(sep) tail
say 'adjacency=' || tail
return
noargs:
arg missing
say 'noargs=' || arg() || '|' || missing
return
fun:
arg value
return arg(1)
