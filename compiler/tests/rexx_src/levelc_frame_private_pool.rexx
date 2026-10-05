value = 'outer'
call inner
say value
exit
inner:
procedure
value = 'private'
say value
return
