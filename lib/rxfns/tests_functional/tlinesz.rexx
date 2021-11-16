/* rexx test linesize bif */
options levelb
say 'Look for LINESIZE OK'

if linesize() \= 0 then say 'failed in test 1'

say 'LINESIZE OK'

linesize: procedure = .int
  arg expose string1 = ' '


