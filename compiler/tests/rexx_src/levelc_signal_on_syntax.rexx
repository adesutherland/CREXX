options levelc
signal on syntax name caught
say 'before'
say substr('abc',0)
say 'skip'
caught:
say 'caught=' || rc || '|' || sigl
return
