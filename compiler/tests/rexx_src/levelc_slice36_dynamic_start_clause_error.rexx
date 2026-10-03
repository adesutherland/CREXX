options levelc
value='bad'
control=10
do control=value to endpoint() by step() for count()
  say 'unreachable'
end control
exit
endpoint: procedure expose control
say 'to' control
return 'bad'
step: procedure
say 'unreachable-step'
return 1
count: procedure
say 'unreachable-count'
return 0
