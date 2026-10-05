options levelc
n = 3
shared = 0
call recursive
say 'recursive=' || n || '|' || shared
call transfer
say 'transfer=' || shared
call witharg 'child'
say 'arg=' || shared
exit
recursive:
  /* This comment is not an executed instruction. */
  procedure expose n shared
  if n = 0 then return
  n = n - 1
  shared = shared + 1
  call recursive
  shared = shared + 1
  return
transfer:
  procedure expose shared
  shared = 'before'
  signal finished
  shared = 'skipped'
finished:
  shared = 'after'
  return
witharg:
  procedure expose shared
  arg local
  shared = shared || ':' || local
  return
