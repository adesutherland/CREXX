options levelc
say 'before'
nop
if 0 then say 'bad'
else nop
if 1 then nop
else say 'bad'
do
  nop
  if 1 then nop
end
say marker()
say 'after'
exit
marker:
procedure
nop
if 1 then nop
do
  nop
end
return 'middle'
