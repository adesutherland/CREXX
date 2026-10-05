options levelc
say 'pre=' || condition() || '|' || condition('C') || '|' || condition('D') || '|' || condition('E') || '|' || condition('I') || '|' || condition('S')
signal on novalue name caught
say missing
exit
caught:
say 'caught=' || condition() || '|' || condition('C') || '|' || condition('D') || '|' || condition('E') || '|' || condition('I') || '|' || condition('S')
call nested
say 'restored=' || condition('C') || '|' || condition('D') || '|' || condition('S')
signal on novalue
say 'enabled=' || condition('S')
return
nested:
procedure
say 'nested=' || condition('C') || '|' || condition('D') || '|' || condition('S')
return
