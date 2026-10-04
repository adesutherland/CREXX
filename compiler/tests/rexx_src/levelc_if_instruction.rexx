options levelc
count=0
value='old'
if 1 then value='new'; else value='bad'
if 0 then drop value; else nop
if 1 then parse value 'red blue' with first second
if 1 then call mark
if 0 then options local(); else options 'unknown'
if 1 then do
  say first second value
end
if 1 then select
  when 1 then say count
  otherwise say 'bad select'
end
do i=1 to 3
  if i=1 then iterate
  if i=3 then leave
  say i
end
select
  when 1 then if 1 then say 'select-if'; else say 'bad nested'
  otherwise say 'bad otherwise'
end
say local()
if 1 then ; label: say 'label'
if 0 then if 1 then say 'bad inner'; else say 'bad inner else'; else say 'outer'
exit

mark: procedure expose count
  count=count+1
  return

local: procedure
  if 1 then return 'local'
  else return 'bad local'
  return 'unreachable'
