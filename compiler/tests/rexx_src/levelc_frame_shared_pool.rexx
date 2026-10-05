value = 'outer'
call outer
say value
exit
outer:
value = 'changed'
call inner
return
inner:
value = value || '-inner'
return
