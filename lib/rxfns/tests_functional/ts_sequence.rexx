/* rexx test sequence */
options levelb
errors=0

if sequence('i','j') \= 'ij' then do
  errors=errors+1
  say 'SEQUENCE failed in test 1'
end


if sequence('i','y') \= 'ijklmnopqrstuvwxy' then do
  errors=errors+1
  say 'SEQUENCE failed in test 2'
end

return errors<>0

/* function prototypes */
sequence: procedure = .string
arg expose startrange = .string, endrange = .string

