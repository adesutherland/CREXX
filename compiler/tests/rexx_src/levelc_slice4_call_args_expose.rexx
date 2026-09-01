options levelc

a = 1
call add 2
say a
exit

add:
procedure expose a
arg amount
a = a + amount
return
