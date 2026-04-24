options levelb
namespace address_callback_host expose address_callback_host
import rxfnsb

address_callback_host: procedure = .int
  buffer = "example-value"
  pool = .standardaddresssandbox

  rc = -1
  address editor "OPEN demo.txt" expose buffer
  if rc \= 0 then return 1
  if buffer \= "native-updated" then return 4

  rc = -1
  address editor
  "CURSOR 7 9"
  if rc \= 0 then return 2

  rc = -1
  address "" "RETURN 42"
  if rc \= 42 then return 3

  pool = .standardaddresssandbox()
  call pool.set("value.3", "native-input")
  rc = -1
  address editor "SANDBOX ROUNDTRIP" sandbox pool
  if rc \= 0 then return 5

  return 0
