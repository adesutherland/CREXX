options levelc
calls = 0
do 0 while tick()
  say 'bad-zero-while'
end
say 'zero-while-calls' calls
do 0 until tick()
  say 'bad-zero-until'
end
say 'zero-until-calls' calls
a = 0
do 5 while a < 2 & tick()
  a = a + 1
  if a > 5 then exit
  say 'while' a
end
say 'after-while' a calls
b = 0
do 2 while b < 5 & tick()
  b = b + 1
  say 'count-while' b
end
say 'after-count-while' b calls
c = 0
do 5 until c >= 2 & tick()
  c = c + 1
  if c > 5 then exit
  if c = 1 then iterate
  say 'until' c
end
say 'after-until' c calls
d = 0
do 2 until d >= 5 & tick()
  d = d + 1
  if d > 5 then exit
  say 'count-until' d
end
say 'after-count-until' d calls
word = 'ab'
idx = 0
do 4 while substr(word, idx + 1, 1) = 'a' | substr(word, idx + 1, 1) = 'b'
  idx = idx + 1
  if idx > 4 then exit
  say 'substr' idx
end
say 'after-substr' idx
e = 0
do 5 until e >= 4 & tick()
  e = e + 1
  if e > 6 then exit
  do
    if e = 1 then iterate
  end
  if e = 3 then leave
  say 'body' e
end
say 'after-body' e calls
say p()
exit
tick:
procedure expose calls
calls = calls + 1
return 1
p:
procedure
x = 0
do 3 while x < 2 & 1
  x = x + 1
end
return x
