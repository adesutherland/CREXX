/* rexx test linesize bif */
options levelb
if linesize() \= 0 then say 'failed in test 1'

return

linesize: procedure = .int
  arg expose string1 = ' '


