/* rexx test address bif */
options levelb

errors=0

out = .string[]
err = .string[]

address cmd 'echo #42'  output out error err
if out.1 <> '#42' then errors=errors+1

return errors<>0
