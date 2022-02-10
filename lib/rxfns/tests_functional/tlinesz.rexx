/* rexx test linesize bif */
options levelb
errors=0

if linesize() \= 0 then do
  errors=errors+1
  say 'LINESIZE failed in test 1'
end

return errors<>0
linesize: procedure = .int
  arg expose string1 = ' '


