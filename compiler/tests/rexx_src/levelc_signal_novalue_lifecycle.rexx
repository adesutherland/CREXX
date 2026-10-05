options levelc
signal on novalue name caught
say 'before'
say unbound
say 'bad'
exit
caught:
say 'caught=' || sigl || '|' || rc
say after_first
signal on novalue name again
say another
return
again:
say 'again=' || sigl || '|' || rc
