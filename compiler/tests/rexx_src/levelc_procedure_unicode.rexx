options levelc
tail = 'é'
u.tail = 'before'
call update
say 'unicode=' || u.tail
exit
update:
  procedure expose tail u.tail
  u.tail = 'after'
  return
