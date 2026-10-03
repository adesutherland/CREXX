options levelc
calls=0
do i=1 to 2 while tick()
  say 'to' i calls
end i
say 'after-to' i calls
calls=0
do zero=3 to 1 while tick()
  say 'bad-to'
end zero
say 'after-zero-to' zero calls
calls=0
do empty=1 for 0 while tick()
  say 'bad-for'
end empty
say 'after-zero-for' empty calls
calls=0
do k=1 by 2 for 3 while tick()
  say 'for' k calls
end k
say 'after-for' k calls
calls=0
do down=3 to 1 by -1 while tick()
  if down=2 then iterate DoWn
  say 'down' down calls
end down
say 'after-down' down calls
calls=0
do left=1 to 3 while tick()
  if left=2 then leave LeFt
  say 'left' left calls
end left
say 'after-left' left calls
calls=0
do both=1 to 3 for 2 while tick()
  say 'both' both calls
end both
say 'after-both' both calls
say local()
exit
tick: procedure expose calls
calls=calls+1
return calls<4
local: procedure
n=0
do scoped=1 for 2 while n<2
  say 'local' scoped n
  n=n+1
end scoped
return scoped
