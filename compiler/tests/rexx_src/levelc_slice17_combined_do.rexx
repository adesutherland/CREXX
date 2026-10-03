options levelc
n = 0
do 5 while n < 3
  n = n + 1
  say 'while' n
end
say 'after-while' n
do 0 while 2
  say 'bad-zero-while'
end
say 'zero-while'
n = 0
do 5 until n >= 3
  n = n + 1
  if n = 1 then iterate
  say 'until' n
end
say 'after-until' n
do 0 until 2
  say 'bad-zero-until'
end
say 'zero-until'
guard = 0
do 5 until 1
  guard = guard + 1
  if guard > 4 then exit
  do
    iterate
  end
  say 'bad-iterate'
end
say 'after-iterate' guard
do 3 while 1
  select
    when 1 then leave
    otherwise say 'bad-select'
  end
  say 'bad-leave'
end
say 'after-leave'
say p()
exit
p:
procedure
x = 0
do 5 while x < 2
  x = x + 1
end
return x
