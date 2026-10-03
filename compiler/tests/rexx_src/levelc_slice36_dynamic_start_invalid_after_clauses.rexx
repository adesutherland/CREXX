options levelc
value='bad'
control=10
do control=value to endpoint() by step() for count()
  say 'unreachable'
end control
exit
endpoint: procedure expose control
say 'to' control
return 2
step: procedure expose control
say 'by' control
return 1
count: procedure expose control
say 'for' control
return 0
