options levelc
n = 0
do while n > 0
  say 'bad-zero'
end
say 'zero'
do while n < 3
  n = n + 1
  say 'count' n
end
i = 0
do while i < 4
  i = i + 1
  do
    if i = 2 then iterate
  end
  select
    when i = 4 then leave
    otherwise nop
  end
  say 'body' i
end
say 'after' i
do 2
  j = 0
  do while j < 2
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
do while x < 2
  x = x + 1
end
return x
