options levelc
signal on error name signalignored
call seed
call on error name levelc_call_external_handler
nop
say 'after=' || result || '|' || .result || '|' || sigl
exit
seed:
  return 'prior'
signalignored:
  say 'unexpected-signal'
