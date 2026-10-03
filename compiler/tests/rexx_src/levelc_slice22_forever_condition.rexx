options levelc
calls = 0
n = 0
do forever while n < 2 & tick()
  n = n + 1
  if n > 4 then exit
  say 'while' n
end
say 'after-while' n calls
do forever while 0 & tick()
  say 'bad-zero'
end
say 'after-zero' calls
u = 0
do forever until u >= 2 & tick()
  u = u + 1
  if u > 4 then exit
  if u = 1 then iterate
  say 'until' u
end
say 'after-until' u calls
do forever until 1 & tick()
  say 'once'
end
say 'after-once' calls
word = 'ab'
idx = 0
do forever while substr(word, idx + 1, 1) = 'a' | substr(word, idx + 1, 1) = 'b'
  idx = idx + 1
  if idx > 4 then exit
  say 'substr' idx
end
say 'after-substr' idx
i = 0
do forever until i >= 4 & 1
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
return 1
p:
procedure
x = 0
do forever while x < 2 & 1
  x = x + 1
  if x > 4 then return 'bad'
end
return x
