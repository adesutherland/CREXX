options levelc
say 'a'
say
if 0 then say
if 1 then say
do
  say 'b'
  say
end
say 'call=' || local()
say
counter=0
say 'expr=' || next()
say 'count=' || counter
say 'expr=' || next()
say 'count=' || counter
say 'z'
exit

local: procedure
say
return 'ok'

next: procedure expose counter
counter=counter+1
say 'inside=' || counter
return counter
