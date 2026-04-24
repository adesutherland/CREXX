options levelb
namespace address_cms_host expose address_cms_host
import rxfnsb

address_cms_host: procedure = .int
  errors = 0

  cms_out = .string[]
  list_out = .string[]
  type_out = .string[]
  cmd = .string

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

  return errors
