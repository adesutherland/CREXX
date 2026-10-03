options levelc
i=10
calls=0
do i=1 to tovalue() by byvalue() for 0
 say 'bad'
end i
say 'first-after' i calls
i=10
calls=0
do i=1 by byvalue() to tovalue() for 0
 say 'bad'
end i
say 'second-after' i calls
step=2
do count=1 to 5 by step
  say 'up' count step
  step=1
end count
say 'after-up' count step
step=-1
do down=3 to 1 by step
  say 'down' down step
  step=1
end down
say 'after-down' down step
step=0
guard=0
do zero=1 to 2 by step
  guard=guard+1
  if guard>3 then exit
  if guard=2 then leave zero
  say 'zero' zero
end zero
say 'after-zero' zero
step=2
do no_to=1 by step for 3
  say 'no-to' no_to
end no_to
say 'after-no-to' no_to
step=1
do checked=1 to 4 by step while checked<3
  say 'while' checked
end checked
say 'after-while' checked
step=1
do after=1 to 4 by step until after=2
  say 'until' after
end after
say 'after-until' after
step=1
loopguard=0
do outer=1 to 3 by step
  loopguard=loopguard+1
  if loopguard>6 then exit
  do inner=1 to 2
    if inner=1 then iterate OuTeR
  end inner
end outer
say 'after-outer' outer
say local()
exit
tovalue: procedure expose i calls
calls=calls+1
say 'to' i calls
return 3
byvalue: procedure expose i calls
calls=calls+1
say 'by' i calls
return 1
local: procedure
step=2
do scoped=1 by step for 2
  say 'local' scoped
end scoped
return scoped
