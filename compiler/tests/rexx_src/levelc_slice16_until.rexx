options levelc
n = 0
do until n = 1
  n = n + 1
  say 'once' n
end
say 'after-once'
guard = 0
do until 1
  guard = guard + 1
  if guard > 4 then exit
  do
    iterate
  end
  say 'bad'
end
say 'after-iterate' guard
i = 0
do until i >= 3
  i = i + 1
  if i = 1 then do
    iterate
  end
  select
    when i = 3 then leave
    otherwise nop
  end
  say 'body' i
  if i > 5 then exit
end
say 'after' i
do 2
  j = 0
  do until j >= 2
    j = j + 1
    say 'inner' j
  end
  say 'outer'
end
say p()
exit
p:
procedure
x = 0
do until x >= 2
  x = x + 1
  if x > 4 then return 'bad'
end
return x
