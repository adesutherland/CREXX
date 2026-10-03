options levelc
do outer = 1 to 3
  if outer > 7 then exit
  do inner = 1 to 3
    if inner > 7 then exit
    if inner = 2 then iterate OuTeR
    say 'outer-iter' outer inner
  end inner
  say 'bad-inner'
end outer
say 'after-outer-iter' outer inner
do alpha = 1 to 3
  if alpha > 7 then exit
  do beta = 1 to 3
    if beta > 7 then exit
    if beta = 2 then leave ALPHA
    say 'outer-leave' alpha beta
  end beta
  say 'bad-beta'
end alpha
say 'after-outer-leave' alpha beta
do source = 1 for 2
  if source > 7 then exit
  do target = 1 to 3
    if target > 7 then exit
    do
      if target = 1 then iterate TaRgEt
    end
    say 'inner-iter' source target
  end target
end source
say 'after-inner-iter' source target
do chosen = 1 to 3
  if chosen > 7 then exit
  select
    when chosen = 1 then iterate ChOsEn
    otherwise leave chosen
  end
end chosen
say 'after-select' chosen
say local()
exit
local:
procedure
do top = 1 for 2
  if top > 7 then return 'bad'
  do low = 1 to 3
    if low > 7 then return 'bad'
    if low = 2 then iterate TOP
  end low
end top
return top
