/* rexx test address bif */
options levelb

errors=0

out = .string[]
err = .string[]
cms_out = .string[]
list_out = .string[]
type_out = .string[]
cmd = .string

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

/* ADDRESS env should update the current/default environment */
address cms
cmd = 'CP SET MSG OFF'
cmd
if rc <> 0 then errors=errors+1

address cms 'CP QUERY USERID' output cms_out
if cms_out.1 <> 'CMSUSER' then errors=errors+1

address cms 'LISTFILE' output list_out
if list_out.1 <> 'DEMO EXEC A1' then errors=errors+1

address cms 'TYPE README EXEC' output type_out
if type_out.1 <> 'CMS TYPE DEMO' then errors=errors+1

address cms 'CP SET MSG ON'
if rc <> 0 then errors=errors+1

cmd = 'CP QUERY USERID'
cmd
if rc <> 0 then errors=errors+1

address system
'echo SYSTEM_AFTER_CMS'
if rc <> 0 then errors=errors+1

return errors<>0
