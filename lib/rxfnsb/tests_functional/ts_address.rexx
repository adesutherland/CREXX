/* rexx test address bif */
options levelb

errors=0

out = .string[]
err = .string[]

address cmd 'echo #42'  output out error err
if out.1 <> '#42' then errors=errors+1

/* This should print OK - to the console - we are just testing the address bit returns ok */
rc = 1
'echo OK'
if rc <> 0 then errors=errors+1

/* Test explicit address shell */
rc = 1
address shell 'echo Hello Shell'
if rc <> 0 then errors=errors+1

return errors<>0
