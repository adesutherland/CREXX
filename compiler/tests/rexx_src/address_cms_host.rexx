options levelb
namespace address_cms_host expose address_cms_host
import rxfnsb

address_cms_host: procedure = .int
  errors = 0

  cms_out = .string[]
  quoted_out = .string[]
  list_out = .string[]
  type_out = .string[]
  buffer = .string
  pool = .standardaddresssandbox
  items = .string[]

  address cms
  "CP SET MSG OFF"
  if rc <> 0 then errors = errors + 1

  address cms "CP QUERY USERID" output cms_out
  if cms_out.1 <> "CMSUSER" then errors = errors + 1

  address "CP QUERY USERID" output quoted_out
  if rc <> 0 then errors = errors + 1
  if quoted_out.1 <> "CMSUSER" then errors = errors + 1

  address cms "LISTFILE" output list_out
  if list_out.1 <> "DEMO EXEC A1" then errors = errors + 1

  address cms "TYPE README EXEC" output type_out
  if type_out.1 <> "CMS TYPE DEMO" then errors = errors + 1

  address cms "CP SET MSG ON"
  if rc <> 0 then errors = errors + 1

  "CP QUERY USERID"
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
  call pool.set("value.3", "current-env-input")
  address cms
  "SANDBOX ROUNDTRIP" sandbox pool
  if rc <> 0 then errors = errors + 1
  if pool.get("result") <> "current-env-input:cms" then errors = errors + 1

  pool = .standardaddresssandbox()
  call pool.set("value.3", "quoted-current-env-input")
  address "SANDBOX ROUNDTRIP" sandbox pool
  if rc <> 0 then errors = errors + 1
  if pool.get("result") <> "quoted-current-env-input:cms" then errors = errors + 1

  items[1] = "cms-one"
  items[2] = "cms-two"
  address cms "EXPOSE ARRAY" expose items[]
  if rc <> 0 then errors = errors + 1
  if items[1] <> "cms-one" then errors = errors + 1
  if items[2] <> "cms-two-updated" then errors = errors + 1
  if items[3] <> "cms-three" then errors = errors + 1
  if items.0 <> 3 then errors = errors + 1

  items = .string[]
  items[1] = "cms-one"
  items[2] = "cms-two"
  address "EXPOSE ARRAY" expose items[]
  if rc <> 0 then errors = errors + 1
  if items[1] <> "cms-one" then errors = errors + 1
  if items[2] <> "cms-two-updated" then errors = errors + 1
  if items[3] <> "cms-three" then errors = errors + 1
  if items.0 <> 3 then errors = errors + 1

  return errors
