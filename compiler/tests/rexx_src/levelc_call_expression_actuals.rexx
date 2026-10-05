options levelc
order = ''
call top 'aé🙂', 2
say 'order=' || order
call relay 'é', 'FF'x, 1 + 2
call edges , side('C'),;
say 'order=' || order
call empty
exit

top: procedure expose order
arg text, depth
say 'enter=' || text || '|' || depth
if depth > 0 then do
  next = depth - 1
  call top arg(1), next
end
call relay side('A'),,side('B')
arg again
say 'leave=' || again || '|' || arg(2)
return

side: procedure expose order
arg token
order = order || token
return token

relay: procedure
arg first, middle, third
say 'relay=' || first || '|' || middle || '|' || third
return

edges: procedure
say 'edges=' || arg() || '|' || arg(1) || '|' || arg(2) || '|' || arg(3)
return

empty: procedure
say 'empty=' || arg()
return
