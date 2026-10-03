options levelc
guard=0
do plain=1
  guard=guard+1
  if guard>5 then exit
  if plain=3 then leave plain
  say 'plain' plain
end plain
say 'after-plain' plain
guard=0
do next=1 by 2
  guard=guard+1
  if guard>5 then exit
  if next=1 then iterate NeXt
  if next>5 then leave next
  say 'next' next
end next
say 'after-next' next
guard=0
do down=3 by -1
  guard=guard+1
  if guard>5 then exit
  if down<1 then leave down
  say 'down' down
end down
say 'after-down' down
guard=0
do changed=1
  guard=guard+1
  if guard>5 then exit
  if changed=1 then changed=3
  if changed>4 then leave changed
  say 'changed' changed
end changed
say 'after-changed' changed
calls=0
do checked=1 while tick()
  say 'while' checked calls
end checked
say 'after-while' checked calls
calls=0
do after=1 until done()
  say 'until' after calls
end after
say 'after-until' after calls
say local()
exit
tick: procedure expose calls
calls=calls+1
return calls<3
done: procedure expose calls
calls=calls+1
return calls=2
local: procedure
guard=0
do scoped=1
  guard=guard+1
  if guard>5 then return 'bad'
  if scoped=2 then leave scoped
end scoped
return scoped
