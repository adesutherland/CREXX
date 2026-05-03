/* rexx test address bif */
options levelb

errors=0

out = .string[]
err = .string[]
cmd_out = .string[]
path_out = .string[]

address command 'echo #42'  output out error err
if out.1 <> '#42' then errors=errors+1

address cmd 'echo CMD_OK' output cmd_out
if cmd_out.1 <> 'CMD_OK' then errors=errors+1

address path 'echo PATH_OK' output path_out
if path_out.1 <> 'PATH_OK' then errors=errors+1

/* This should print OK - to the console - we are just testing the address bit returns ok */
rc = 1
'echo OK'
if rc <> 0 then errors=errors+1

/* Test explicit address shell */
rc = 1
address shell 'echo Hello Shell'
if rc <> 0 then errors=errors+1

address command
'echo COMMAND_AFTER_COMMAND'
if rc <> 0 then errors=errors+1

return errors<>0
