options levelc
do plain = 1 for 2
  if plain > 8 then exit
  say 'plain' plain
end plain
say 'after-plain' plain
do down = 3 by -1 for 2
  if down < -5 then exit
  say 'down' down
end down
say 'after-down' down
do up = 1 for 2 by 2
  if up > 9 then exit
  say 'up' up
end up
say 'after-up' up
do zero = 1 for 0
  say 'bad-zero'
end zero
say 'after-zero' zero
do changed = 1 for 2
  if changed = 1 then changed = 2
  if changed > 8 then exit
  say 'changed' changed
end changed
say 'after-changed' changed
do iter = 1 for 3
  if iter > 8 then exit
  if iter = 2 then iterate
  say 'iter' iter
end iter
say 'after-iter' iter
do left = 1 for 4
  if left > 8 then exit
  do
    if left = 2 then leave
  end
  say 'left' left
end left
say 'after-left' left
say local()
exit
local:
procedure
do value = 1 for 2
  if value > 8 then return 'bad'
end value
return value
