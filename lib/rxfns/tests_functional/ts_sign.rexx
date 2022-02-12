/* rexx */
options levelb
errors=0

if sign(3.14) \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 1'
end
  
if sign("3.14") \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 2'
end

if sign(-3.14) \= -1 then do
  errors=errors+1
  say 'SIGN Failed in test 3'
end

if sign(-"3.14") \= -1 then do
  errors=errors+1
  say 'SIGN Failed in test 4'
end


if sign(0) \= 0 then do
  errors=errors+1
  say 'SIGN Failed in test 5'
end

if sign("0") \= 0 then do
  errors=errors+1
  say 'SIGN Failed in test 6'
end

if sign("+0") \= 0 then do
  errors=errors+1
  say 'SIGN Failed in test 7'
end

if sign("-0") \= 0 then do
  errors=errors+1
  say 'SIGN Failed in test 8'
end

if sign("0e-12") \= 0 then do
  errors=errors+1
  say 'SIGN Failed in test 9'
end

if sign("0E999999999") \= 0 then do
  errors=errors+1
  say 'SIGN Failed in test 10'
end

/* if sign() \= 0 then do */
/*   errors=errors+1 */
/*   say 'SIGN Failed in test 11' */
/* end */

if sign(1) \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 11'
end

if sign(+1.) \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 12'
end

if sign(-2.0000000000000000) \= -1 then do
  errors=errors+1
  say 'SIGN Failed in test 13'
end

if sign(2.0000000000000000) \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 14'
end

if sign(2.00000000000000005) \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 15'
end

if sign(333333333333333333) \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 16'
end

if sign(-4444444444444444444e12) \= -1 then do
  errors=errors+1
  say 'SIGN Failed in test 17'
end

/* if sign(9e999999999) \= 1 then do */
/*   errors=errors+1 */
/*   say 'SIGN Failed in test 18' */
/* end */

if sign("123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789") \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 19'
end

if sign('3132'x) \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 20'
end

if sign('1'||'23'||'456'||'7890') \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 21' 
end

if sign('17'-'1.e2') \= -1 then do
  errors=errors+1
  say 'SIGN Failed in test 22' 
end

/* this waits for numeric digits 9
if sign(17+4-21.000000000000005) \= 0 then do
  errors=errors+1
  say 'SIGN Failed in test 23' 
   end */

if sign(-"123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789") \= -1 then do
  errors=errors+1
  say 'SIGN Failed in test 24' 
end

if sign(- -"123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789123456789") \= 1 then do
  errors=errors+1
  say 'SIGN Failed in test 24' 
end

return errors<>0

sign: procedure = .int
  arg expose number = .float

