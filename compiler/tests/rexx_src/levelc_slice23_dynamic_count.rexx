options levelc
count = 3
i = 0
do count
  i = i + 1
  count = 0
  say 'scalar' i
end
say 'after-scalar' i count
do count
  say 'bad-zero'
end
say 'after-zero'
count = '2.0'
j = 0
do count
  j = j + 1
  say 'decimal' j
end
count = '2E0'
j = 0
do count
  j = j + 1
  say 'exponent' j
end
calls = 0
k = 0
do tick()
  k = k + 1
  say 'call' k
end
say 'after-call' k calls
m = 0
do length('abc')
  m = m + 1
  say 'length' m
end
s = 0
do substr('2x', 1, 1)
  s = s + 1
  say 'substr-count' s
end
count = 5
i = 0
do count
  i = i + 1
  if i > 6 then exit
  do
    if i = 1 then iterate
  end
  if i = 3 then leave
  say 'body' i
end
say 'after-body' i
say p()
exit
tick:
procedure expose calls
calls = calls + 1
return 2
p:
procedure
times = 2
x = 0
do times
  x = x + 1
end
return x
