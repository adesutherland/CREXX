options levelc
calls=0
do i=1 to 3 until stop()
  say 'to' i calls
end i
say 'after-to' i calls
calls=0
do zero=3 to 1 until stop()
  say 'bad-to'
end zero
say 'after-zero-to' zero calls
calls=0
do empty=1 for 0 until stop()
  say 'bad-for'
end empty
say 'after-zero-for' empty calls
calls=0
do foronly=1 by 2 for 3 until stop()
  say 'for' foronly calls
end foronly
say 'after-for' foronly calls
calls=0
do counted=1 for 2 until never()
  say 'counted' counted calls
end counted
say 'after-counted' counted calls
calls=0
do down=3 to 1 by -1 until stop()
  if down=3 then iterate DoWn
  say 'down' down calls
end down
say 'after-down' down calls
calls=0
do left=1 to 3 until stop()
  leave LeFt
end left
say 'after-left' left calls
calls=0
do both=1 to 3 for 2 until stop()
  say 'both' both calls
end both
say 'after-both' both calls
say local()
exit
stop: procedure expose calls
calls=calls+1
return calls=2
never: procedure expose calls
calls=calls+1
return 0
local: procedure
n=0
do scoped=1 for 3 until n=1
  say 'local' scoped n
  n=n+1
end scoped
return scoped
