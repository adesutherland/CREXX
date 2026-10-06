options levelc
trace r
x = 1 + 2
say x
trace value 'intermediates'
nul = x2c('00')
say c2x(nul)
s. = 'base'
i = 1
s.i = '漢🙂'
y = s.i
say y
parse value 'a b' with a b
say a b
call child 'é'
say 'mode' trace()
old = trace('O')
say 'off' old
exit
child:
  arg p
  say p
  trace o
  return
