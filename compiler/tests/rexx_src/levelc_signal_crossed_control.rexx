options levelc
signal on syntax name caught
count = 0
do outer = 1 to 2
  do inner = 1 to 2
    count = count + 1
    signal after_loop
  end
end
say 'not reached'
exit
after_loop:
say 'branch=' || count || '|' || outer || '|' || inner || '|' || sigl
do next = 1 to 2
  if next = 1 then say substr('abc', 0)
end
return
caught:
say 'caught=' || rc || '|' || sigl || '|' || condition('C') || '|' || condition('S')
do later = 1 to 2
  say 'later=' || later
end
return
