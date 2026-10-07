options levelc
shared = 'caller'
call levelc_call_external_middle x2c('410042'),, 'é🙂'
say 'result=' || c2x(result) || '|' || c2x(.result)
say 'caller-pool=' || shared
call levelc_call_external_leaf
say 'void=' || symbol('RESULT') || '|' || symbol('.RESULT')
call typedentry
say 'local=' || result
call 'typedentry'
say 'bg=' || result || '|' || .result
say 'caller-source=' || sourceline(2)
exit
typedentry:
  return 'LOCAL'
