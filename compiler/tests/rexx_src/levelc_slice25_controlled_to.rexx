options levelc
do i = 1 to 3
  if i > 8 then exit
  say 'basic' i
end i
say 'after-basic' i
do zero = 3 to 1
  say 'bad-zero'
end zero
say 'after-zero' zero
do changed = 1 to 4
  if changed = 1 then changed = 2
  if changed > 8 then exit
  say 'changed' changed
end changed
say 'after-changed' changed
do iter = 1 to 3
  if iter > 8 then exit
  if iter = 2 then iterate
  say 'iter' iter
end iter
say 'after-iter' iter
do left = 1 to 3
  if left > 8 then exit
  do
    if left = 2 then leave
  end
  say 'left' left
end left
say 'after-left' left
do outer = 1 to 2
  if outer > 5 then exit
  do inner = 1 to 2
    if inner > 5 then exit
    if inner = 1 then iterate
    say 'nested' outer inner
  end inner
end outer
say 'after-nested' outer inner
say local()
exit
local:
procedure
do value = 1 to 2
  if value > 8 then return 'bad'
end value
return value
