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

return errors<>0

sign: procedure = .int
  arg expose number = .float

