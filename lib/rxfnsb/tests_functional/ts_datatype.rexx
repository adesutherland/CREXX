/* test the datatype built-in function */
options levelb
import rxfnsb


/* DATATYPE */
errors=0
/* These from the Rexx book. */

if datatype('1') \= 'NUM'      then do
  errors=errors+1
  say 'ts_datatype failed in test 1' /* datatype('1') */
end
if datatype(' 12 ') \= 'NUM'      then do
   errors=errors+1
   say 'ts_datatype failed in test 2' datatype(' 12 ') \= 'NUM'
end
if datatype('') \= 'CHAR'         then do
   errors=errors+1
   say 'ts_datatype failed in test 2'
 end
  
 if datatype('123*') \= 'CHAR'     then do
    errors=errors+1
    say 'ts_datatype failed in test 3'
  end

return errors <> 0