options levelc
address crexx with output stem out.
'echo Literal'
say 'literal=' || out.0 || '/' || out.1 || '/' || address('O')
command = 'echo Variable'
command
say 'variable=' || out.0 || '/' || out.1
command.tail = 'echo Compound'
command.tail
say 'compound=' || out.0 || '/' || out.1
'echo ' || 'Joined'
say 'joined=' || out.0 || '/' || out.1
'echo Héllo🙂'
say 'unicode=' || out.0 || '/' || out.1
prefix = 'echo'
prefix ('Spaced')
say 'spaced=' || out.0 || '/' || out.1
times = 0
make_command()
say 'function=' || times || '/' || out.0 || '/' || out.1
if 1 then 'echo If'
say 'if=' || out.1
select
  when 1 then 'echo Select'
  otherwise say 'unexpected'
end
say 'select=' || out.1
do index = 1 to 1
  'echo Loop'
end
say 'loop=' || out.1
call local
say 'local=' || out.0 || '/' || out.1 || '/' || address()
call recurse 2
say 'recursive=' || out.0 || '/' || out.1 || '/' || address()
address bogus
'ignored'
say 'unknown=' || rc || '/' || .rc || '/' || .rs || '/' || address()
address
say 'restored=' || address() || '/' || address('O')
address crexx with output append stem out.
'echo Appended'
say 'append=' || out.0 || '/' || out.1 || '/' || out.2
exit
make_command:
  times = times + 1
  return 'echo Once'
local: procedure expose out.
  'echo Local'
  return
recurse: procedure expose out.
  arg depth
  'echo Rec' || depth
  if depth > 1 then call recurse depth - 1
  return
