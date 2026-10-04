options levelc
n = 0
do 4
  n = n + 1
  if n = 2 then iterate
  say 'counted' n
end
say 'counted-state' n

n = 0
do forever
  n = n + 1
  if n = 2 then iterate
  if n = 4 then leave
  say 'forever' n
end
say 'forever-state' n

i = 0
do while i < 3
  i = i + 1
  if i = 2 then iterate
  say 'while' i
end
say 'while-state' i

i = 0
do until i >= 3
  i = i + 1
  if i = 2 then iterate
  say 'until' i
end
say 'until-state' i

do ctl = 1 to 5 by 2
  if ctl = 3 then iterate
  say 'controlled' ctl
end ctl
say 'controlled-state' ctl

do limited = 1 to 8 for 3
  if limited = 2 then iterate
  say 'for' limited
end limited
say 'for-state' limited

do checked = 1 to 5 by 2 until checked >= 3
  if checked = 1 then iterate
  say 'until-step' checked
end checked
say 'until-step-state' checked

do guarded = 1 to 5 while guarded < 4
  if guarded = 2 then iterate
  say 'while-step' guarded
end guarded
say 'while-step-state' guarded

do limited_until = 1 to 9 for 3 until limited_until = 2
  if limited_until = 1 then iterate
  say 'for-until' limited_until
end limited_until
say 'for-until-state' limited_until

do outer = 1 to 3
  say 'outer-visit' outer
  do inner = 1 to 2
    do
      if inner = 1 then iterate OuTeR
    end
    say 'bad-inner'
  end inner
  say 'bad-outer'
end outer
say 'outer-state' outer inner

do same = 1 to 2
  do same = 1 to 2
    if same = 1 then iterate SAME
    say 'same-inner' same
  end same
  say 'same-outer' same
end same
say 'same-state' same

tail = 'a'
do stem.tail = 1 to 3
  if stem.tail = 1 then iterate STEM.TAIL
  say 'compound' stem.tail
end stem.tail
say 'compound-state' stem.tail

do 2
  select
    when 1 then iterate
    otherwise say 'bad-select'
  end
  say 'bad-after-select'
end
say 'select-done'

say 'local' local()
exit

local:
procedure
do p = 1 to 3
  do q = 1 to 2
    if q = 1 then iterate p
  end q
end p
return p
