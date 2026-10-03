options levelc
n = 0
do 3
  n = n + 1
  if n = 2 then leave
  say 'leave' n
end
say 'after-leave' n
m = 0
do 3
  m = m + 1
  do
    if m < 3 then iterate
  end
  say 'iterate' m
end
do 3
  select
    when 1 then leave
    otherwise say 'bad'
  end
  say 'bad'
end
say 'after-select-leave'
do 2
  select
    when 1 then iterate
    otherwise say 'bad'
  end
  say 'bad'
end
say 'after-select-iterate'
do 2
  do 3
    if 1 then leave
    say 'bad'
  end
  say 'outer'
end
say p()
exit
p:
procedure
x = 0
do 3
  x = x + 1
  if x = 2 then leave
end
return x
