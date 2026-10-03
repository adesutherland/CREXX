options levelc
calls = 0
do until 1 & tick()
  say 'once'
end
say 'first-calls' calls
k = 0
do until k >= 2 & tick()
  k = k + 1
  if k > 5 then exit
  if k = 1 then iterate
  say 'tick-body' k
end
say 'calls' calls
word = 'abcd'
idx = 0
do until substr(word, idx, 1) = 'b'
  idx = idx + 1
  if idx > 4 then exit
  say 'substr' idx
end
say 'after-substr' idx
stem.1 = 'no'
stem.2 = 'yes'
index = 0
do until stem.index = 'yes'
  index = index + 1
  if index > 4 then exit
  say 'tail' index
end
say 'after-tail' index
j = 0
do until j >= 4 & 1
  j = j + 1
  if j > 6 then exit
  do
    if j = 1 then iterate
  end
  if j = 3 then leave
  say 'body' j
end
say 'after-body' j
say p()
exit
tick:
procedure expose calls
calls = calls + 1
return 1
p:
procedure
x = 0
do until x >= 2 & 1
  x = x + 1
  if x > 4 then return 'bad'
end
return x
