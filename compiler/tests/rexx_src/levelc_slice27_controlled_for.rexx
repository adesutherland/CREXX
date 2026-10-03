options levelc
do simple = 1 to 5 for 2
  if simple > 8 then exit
  say 'simple' simple
end simple
say 'after-simple' simple
do step = 1 by 2 to 9 for 2
  if step > 12 then exit
  say 'step' step
end step
say 'after-step' step
do down = 3 for 2 to 1 by -1
  if down < -5 then exit
  say 'down' down
end down
say 'after-down' down
do zero = 1 to 5 for 0
  say 'bad-zero'
end zero
say 'after-zero' zero
do changed = 1 to 5 for 2
  if changed = 1 then changed = 2
  if changed > 8 then exit
  say 'changed' changed
end changed
say 'after-changed' changed
do iter = 1 to 5 for 3
  if iter > 8 then exit
  if iter = 2 then iterate
  say 'iter' iter
end iter
say 'after-iter' iter
do left = 1 to 5 for 4
  if left > 8 then exit
  if left = 2 then leave
  say 'left' left
end left
say 'after-left' left
say local()
exit
local:
procedure
do value = 1 to 4 for 2
  if value > 8 then return 'bad'
end value
return value
