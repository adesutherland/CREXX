options levelc
n = 0
do while n < 3 & 1
  n = n + 1
  if n > 5 then exit
  say 'and' n
end
say 'after-and' n
calls = 0
k = 0
do while k < 2 & tick()
  k = k + 1
  if k > 4 then exit
  say 'tick-loop' k
end
say 'calls' calls
text = 'abcd'
idx = 1
do while substr(text, idx, 1) = 'a'
  idx = idx + 1
  if idx > 4 then exit
  say 'substr' idx
end
say 'after-substr' idx
stem.1 = 'go'
stem.2 = 'stop'
index = 1
do while stem.index = 'go'
  index = index + 1
  if index > 4 then exit
  say 'tail' index
end
say 'after-tail' index
i = 0
do while i < 4 & 1
  i = i + 1
  if i > 6 then exit
  do
    if i = 2 then iterate
  end
  if i = 4 then leave
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
do while x < 2 & 1
  x = x + 1
  if x > 4 then return 'bad'
end
return x
