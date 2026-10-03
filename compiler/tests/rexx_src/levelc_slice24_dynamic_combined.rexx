options levelc
count = 0
checks = 0
do count while substr('2', 1, 1)
  say 'bad-zero-while'
end
do count until substr('2', 1, 1)
  say 'bad-zero-until'
end
say 'after-zero' checks
count = 3
n = 0
do count while n < 2 & tick()
  n = n + 1
  count = 0
  if n > 5 then exit
  say 'while' n
end
say 'after-while' n count checks
count = 2
n = 0
do count while n < 5 & tick()
  n = n + 1
  say 'limited-while' n
end
say 'after-limited-while' n checks
count = 3
u = 0
do count until u >= 2 & tick()
  u = u + 1
  if u > 5 then exit
  if u = 1 then iterate
  say 'until' u
end
say 'after-until' u checks
count = 2
u = 0
do count until u >= 5 & tick()
  u = u + 1
  say 'limited-until' u
end
say 'after-limited-until' u checks
word = 'ab'
idx = 0
do substr('3x', 1, 1) while substr(word, idx + 1, 1) = 'a' | substr(word, idx + 1, 1) = 'b'
  idx = idx + 1
  if idx > 4 then exit
  say 'substr' idx
end
say 'after-substr' idx
count_calls = 0
c = 0
do countfn() until c >= 2 & tick()
  c = c + 1
  if c > 4 then exit
  say 'call' c
end
say 'after-call' c count_calls checks
count = 5
i = 0
do count until i >= 4 & tick()
  i = i + 1
  if i > 6 then exit
  do
    if i = 1 then iterate
  end
  if i = 3 then leave
  say 'body' i
end
say 'after-body' i checks
say p()
exit
tick:
procedure expose checks
checks = checks + 1
return 1
countfn:
procedure expose count_calls
count_calls = count_calls + 1
return 2
p:
procedure
limit = 3
x = 0
do limit while x < 2 & 1
  x = x + 1
  if x > 4 then return 'bad'
end
return x
