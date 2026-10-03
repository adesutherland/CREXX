options levelc
n = 0
say 'before'
do
  n = n + 1
  if n = 1 then do
    say 'nested'
    n = n + 1
  end
  do
    say n
  end
end
say n
if 1 then do
  say 'arm'
end
else say 'bad'
do
end
say mark()
exit
mark:
procedure expose n
do
  n = n + 1
end
return n
