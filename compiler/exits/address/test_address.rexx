options levelb

main: procedure
  failures = .int
  out = .string[]
  err = .string[]
  cms_out = .string[]
  list_out = .string[]
  type_out = .string[]
  cmd = .string

  failures = 0

  address cmd "echo #42" output out error err
  if out.1 <> "#42" then failures = failures + 1

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

  address system
  "echo SYSTEM_OK"
  if rc <> 0 then failures = failures + 1

  if failures = 0 then say "SUCCESS"
  else say "FAILURES" failures
  return
