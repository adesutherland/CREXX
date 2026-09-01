options levelc

i = 1
items.i = "before"
call bump
say items.i
exit

bump:
procedure expose items.
j = 1
items.j = "after"
return
