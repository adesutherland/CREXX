options levelc

a = 1
call change
say a

change:
procedure expose a
a = a + 2
return
