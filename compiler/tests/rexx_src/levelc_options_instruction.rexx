options levelc
count=0
word='vendor alpha'
options word
options tick()
if 1 then options tick()
if 0 then options tick()
do 2
  options tick()
end
select
  when 1 then options tick()
  otherwise options tick()
end
options 'vendor' || ' beta'
options 'abc' '00'x 'def'
call local
options
say count
say 7//3
exit

tick: procedure expose count
  count=count+1
  return 'unknown word'

local: procedure expose count
  options tick()
  return
