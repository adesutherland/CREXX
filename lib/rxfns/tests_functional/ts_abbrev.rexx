/* rexx test abbrev bif */
options levelb
import rxfnsb

errors=0

if abbrev('quicker','quick') <> 1 then do
  errors=errors+1
  say 'ABBREV failed in test 1'
end
if abbrev('quicker','q',2) <> 0 then do
  errors=errors+1
  say 'ABBREV failed in test 2'
end

if abbrev('quicker','fast') <> 0 then do
  errors=errors+1
  say 'ABBREV failed in test 3'
end

return errors<>0
