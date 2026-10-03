options levelc
say 'before'
do 0
  say 'bad'
end
do 2
  say 'outer'
  do 2
    say 'inner'
  end
end
say 'after'
if 1 then do 2
  say 'if'
end
say work()
exit
work:
procedure
v = 0
do 3
  v = v + 1
end
return v
