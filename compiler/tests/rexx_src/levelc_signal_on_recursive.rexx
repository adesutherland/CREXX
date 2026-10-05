options levelc
signal on syntax name caught
call worker 2
say 'after'
exit
worker:
arg depth
if depth > 0 then do
  next = depth - 1
  call worker next
  say 'up=' || depth
  return
end
say substr('abc',0)
return
caught:
say 'caught=' || rc || '|' || sigl
return
