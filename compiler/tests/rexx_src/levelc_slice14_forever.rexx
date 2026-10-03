options levelc
n = 0
do forever
  n = n + 1
  if n = 2 then iterate
  if n = 4 then leave
  if n > 6 then exit
  say n
end
say 'count' n
guard = 0
do forever
  guard = guard + 1
  if guard > 5 then exit
  do
    leave
  end
  say 'bad'
end
say 'after-simple'
guard = 0
do forever
  guard = guard + 1
  if guard > 5 then exit
  select
    when 1 then leave
    otherwise say 'bad'
  end
  say 'bad'
end
say 'after-select'
do 2
  do forever
    leave
  end
  say 'outer'
end
say p()
exit
p:
procedure
x = 0
do forever
  x = x + 1
  if x = 3 then leave
  if x > 5 then return 'bad'
end
return x
