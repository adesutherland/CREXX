options levelc
say missing
a = 'one'
drop a
say a
a = 'two'
say a
b = 'bee'
c = 'see'
drop b c
say b c
if 1 then drop a
say a
do
  a = 'inside'
  drop a
end
say a
shared = 'outer'
say clear_shared()
say shared
exit
clear_shared:
procedure expose shared
say local
drop shared
return shared
