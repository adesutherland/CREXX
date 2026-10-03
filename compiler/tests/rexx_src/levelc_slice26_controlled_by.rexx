options levelc
do down = 3 to 1 by -1
  if down < -5 then exit
  say 'down' down
end down
say 'after-down' down
do up = 1 by 2 to 4
  if up > 9 then exit
  say 'up' up
end up
say 'after-up' up
do changed = 4 to 1 by -1
  if changed = 4 then changed = 3
  if changed < -5 then exit
  say 'changed' changed
end changed
say 'after-changed' changed
do zero = 1 to 1 by 0
  say 'zero' zero
  leave
end zero
say 'after-zero' zero
do bad_up = 2 to 1 by 1
  say 'bad-up'
end bad_up
say 'after-bad-up' bad_up
do bad_down = 1 by -1 to 3
  say 'bad-down'
end bad_down
say 'after-bad-down' bad_down
do nested = 3 to 2 by -1
  if nested < -5 then exit
  do
    if nested = 3 then iterate
  end
  say 'nested' nested
end nested
say 'after-nested' nested
