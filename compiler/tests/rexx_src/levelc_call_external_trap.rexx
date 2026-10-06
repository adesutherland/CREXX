options levelc
call seed
call on error name levelc_call_external_handler
nop
say 'after=' || result || '|' || .result
exit
seed:
  return 'prior'
