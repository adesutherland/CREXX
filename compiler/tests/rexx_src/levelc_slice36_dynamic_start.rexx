options levelc
i=10
calls=0
do i=startvalue() to tovalue() by byvalue() for zerocount()
  say 'bad-zero'
end i
say 'after-zero' i calls
source=2
do changed=source to 4 for 2
  say 'changed' changed source
  source=9
end changed
say 'after-changed' changed source
start_text='-1.0'
do fractional=start_text to 1
  say 'fractional' fractional
  start_text='bad'
end fractional
say 'after-fractional' fractional start_text
begin=1
do checked=begin to 3 while checked<3
  say 'while' checked
end checked
say 'after-while' checked
begin=1
do ending=begin for 3 until ending=2
  say 'until' ending
end ending
say 'after-until' ending
begin=1
do outer=begin to 3
  do inner=1 to 2
    if inner=1 then iterate outer
  end inner
end outer
say 'after-outer' outer
say local()
exit
startvalue: procedure expose i calls
calls=calls+1
say 'start' i calls
return 1
tovalue: procedure expose i calls
calls=calls+1
say 'to' i calls
return 3
byvalue: procedure expose i calls
calls=calls+1
say 'by' i calls
return 1
zerocount: procedure expose i calls
calls=calls+1
say 'for' i calls
return 0
local: procedure
begin=1
do scoped=begin for 2
  say 'local' scoped
end scoped
return scoped
