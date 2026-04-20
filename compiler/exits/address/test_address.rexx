options levelb

main: procedure
  failures = .int
  out = .string[]
  err = .string[]
  cmd_out = .string[]
  path_out = .string[]
  cms_out = .string[]
  list_out = .string[]
  type_out = .string[]
  cmd = .string

  failures = 0

  address command "echo #42" output out error err
  if out.1 <> "#42" then failures = failures + 1

  address cmd "echo CMD_OK" output cmd_out
  if cmd_out.1 <> "CMD_OK" then failures = failures + 1

  address path "echo PATH_OK" output path_out
  if path_out.1 <> "PATH_OK" then failures = failures + 1

  address shell "echo Hello Shell"
  if rc <> 0 then failures = failures + 1

  address cms
  "CP SET MSG OFF"
  if rc <> 0 then failures = failures + 1

  address cms "CP QUERY USERID" output cms_out
  if cms_out.1 <> "CMSUSER" then failures = failures + 1

  address cms "LISTFILE" output list_out
  if list_out.1 <> "DEMO EXEC A1" then failures = failures + 1

  address cms "TYPE README EXEC" output type_out
  if type_out.1 <> "CMS TYPE DEMO" then failures = failures + 1

  address cms "CP SET MSG ON"
  if rc <> 0 then failures = failures + 1

  "CP QUERY USERID"
  if rc <> 0 then failures = failures + 1

  address command
  "echo COMMAND_OK"
  if rc <> 0 then failures = failures + 1

  if failures = 0 then say "SUCCESS"
  else say "FAILURES" failures
  return
