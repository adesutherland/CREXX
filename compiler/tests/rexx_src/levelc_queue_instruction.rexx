options levelc
if 0 then queue 'skip'
queue 'one'
queue 'two'
pull first
pull second
say 'fifo=' || first || '|' || second
queue
pull empty_value
say 'empty=' || length(empty_value)
queue 'a' || '00'x || 'é'
pull ordinal_value
say 'ordinals=' || c2x(ordinal_value)
queue 'é🙂'
pull unicode_value
say 'unicode=' || unicode_value || '|' || length(unicode_value)
queue make()
pull made
say 'effect=' || made
queue 'old'
push 'head'
queue 'tail'
pull head_value
pull old_value
pull tail_value
say 'mixed=' || head_value || '|' || old_value || '|' || tail_value
select
  when 1 then queue 'branch'
  otherwise nop
end
pull selected
say 'selected=' || selected
do index = 1 to 2
  queue 'loop' || index
end
pull loop_first
pull loop_second
say 'loop=' || loop_first || '|' || loop_second
call recursive 2
pull recursive_first
pull recursive_second
say 'recursive=' || recursive_first || '|' || recursive_second
exit

make:
  say 'called'
  return 'mixed Case'

recursive: procedure
  arg depth
  if depth = 0 then return
  queue 'rec' || depth
  call recursive depth - 1
  return
