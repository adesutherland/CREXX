options levelc
return = 8
say 'keyword=' || return
root = '雪'
count = 0
call shared 2
say 'shared=' || result || ';root=' || root
call private
say 'private=' || result || ';secret=' || secret
call bare
say 'bare=' || result
call empty
say 'empty=' || symbol('RESULT') || ';len=' || length(result)
call counted
say 'counted=' || result || ';count=' || count
say 'function=' || c2x(payload()) || ';chars=' || length(payload())
if 0 then return 99
do index = 1 to 2
  select
    when index = 2 then return 0
    otherwise say 'loop=' || index
  end
end
say 'unreachable'
shared:
arg amount
root = root || amount
return root
private:
procedure
secret = 'inside'
return secret
bare:
return
empty:
return ''
counted:
return increment()
increment:
count = count + 1
return count
payload:
return 'é' || x2c('00')
