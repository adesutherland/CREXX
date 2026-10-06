options levelc
call on error name caught
nop
say 'resumed-main'
exit 99
caught:
say 'handler'
exit 12
