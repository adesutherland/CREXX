options levelb
namespace address_cms_host expose address_cms_host
import rxfnsb

address_cms_host: procedure = .int
  errors = 0

  cms_out = .string[]
  list_out = .string[]
  type_out = .string[]
  cmd = .string
  buffer = .string
  pool = .standardaddresssandbox

  address cms
  cmd = "CP SET MSG OFF"
  address "" cmd
  if rc <> 0 then errors = errors + 1

  address cms "CP QUERY USERID" output cms_out
  if cms_out.1 <> "CMSUSER" then errors = errors + 1

  address cms "LISTFILE" output list_out
  if list_out.1 <> "DEMO EXEC A1" then errors = errors + 1

  address cms "TYPE README EXEC" output type_out
  if type_out.1 <> "CMS TYPE DEMO" then errors = errors + 1

  address cms "CP SET MSG ON"
  if rc <> 0 then errors = errors + 1

  cmd = "CP QUERY USERID"
  address "" cmd
  if rc <> 0 then errors = errors + 1

  buffer = "cms-before"
  address cms "SET BUFFER UPDATED" expose buffer
  if rc <> 0 then errors = errors + 1
  if buffer <> "cms-updated" then errors = errors + 1

  pool = .standardaddresssandbox()
  call pool.set("value.3", "cms-input")
  address cms "SANDBOX ROUNDTRIP" sandbox pool
  if rc <> 0 then errors = errors + 1
  if pool.get("RESULT") <> "cms-input:cms" then errors = errors + 1
  if pool.get("Value.4") <> "cms-four" then errors = errors + 1

  address cms "SANDBOX RESPONSE UPDATE" sandbox pool
  if rc <> 0 then errors = errors + 1
  if pool.get("response") <> "cms-response" then errors = errors + 1

  pool = .standardaddresssandbox()
  call pool.set("value.3", "default-input")
  address cms sandbox pool
  "SANDBOX ROUNDTRIP"
  if rc <> 0 then errors = errors + 1
  if pool.get("result") <> "default-input:cms" then errors = errors + 1

  return errors
