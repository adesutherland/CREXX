options levelc
if 0 then push 'skip'
push 'one'
push 'two'
pull first
pull second
say 'lifo=' || first || '|' || second
push
pull empty_value
say 'empty=' || length(empty_value)
push 'a' || '00'x || 'é'
pull ordinal_value
say 'ordinals=' || c2x(ordinal_value)
push 'é🙂'
pull unicode_value
say 'unicode=' || unicode_value || '|' || length(unicode_value)
push make()
pull made
say 'effect=' || made
select
  when 1 then push 'branch'
  otherwise nop
end
pull selected
say 'selected=' || selected
do index = 1 to 2
  push 'loop' || index
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
  push 'rec' || depth
  call recursive depth - 1
  return
