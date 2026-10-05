value = 'main'
call first
say value
exit
first:
value = 'first'
second:
value = value || '-second'
return
