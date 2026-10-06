options levelc
if 0 then pull skipped
pull first second
say 'words=' || first || '|' || second
pull
pull head ':' tail
say 'pattern=' || head || '|' || tail
pull unicode rest
say 'unicode=' || unicode || '|' || rest
pull comma_one comma_two, missing_one missing_two
say 'comma=' || comma_one || '|' || comma_two || '|' || missing_one || '|' || missing_two
pull , empty_second
pull after_empty
say 'empty=' || empty_second || '|' || after_empty
pull position_first 3 position_rest
say 'position=' || position_first || '|' || position_rest
offset = 3
pull dynamic_first =(offset) dynamic_rest
say 'dynamic=' || dynamic_first || '|' || dynamic_rest
tail_name = 'item'
pull stem.tail_name
say 'compound=' || stem.tail_name
if 1 then call nested
pull at_eof
say 'eof=[' || at_eof || ']'
exit

nested: procedure expose shared
  pull shared private
  say 'nested=' || shared || '|' || private
  return
