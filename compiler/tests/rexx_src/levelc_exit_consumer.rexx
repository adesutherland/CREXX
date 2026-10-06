options levelc
rc = 77
call levelc_exit_provider 'value'
say 'value=' || substr(result, 1, 1) || '|' || c2x(substr(result, 2)) || '|' || length(result) || '|' || rc
call levelc_exit_provider 'once'
say 'once=' || result || '|' || rc
call levelc_exit_provider 'empty'
say 'empty=' || symbol('RESULT') || '|' || length(result) || '|' || rc
call levelc_exit_provider 'bare'
say 'bare=' || symbol('RESULT') || '|' || rc
call levelc_exit_eof_provider
say 'eof=' || symbol('RESULT') || '|' || rc
exit 0
