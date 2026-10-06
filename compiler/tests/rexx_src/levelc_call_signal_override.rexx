options levelc
call on error name called
signal on error name transferred
nop
say 'after-override'
exit
called:
  say 'unexpected-call'
  return
transferred:
  say 'unexpected-signal'
  return
