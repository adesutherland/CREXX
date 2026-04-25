options levelb
namespace address_callback_host expose address_callback_host
import rxfnsb

address_callback_host: procedure = .int
  buffer = "example-value"
  pool = .standardaddresssandbox
  items = .string[]

  rc = -1
  address editor "OPEN demo.txt" expose buffer
  if rc \= 0 then return 1
  if buffer \= "native-updated" then return 4

  rc = -1
  address editor
  "CURSOR 7 9"
  if rc \= 0 then return 2

  rc = -1
  "RETURN 42"
  if rc \= 42 then return 3

  pool = .standardaddresssandbox()
  call pool.set("value.3", "native-input")
  rc = -1
  address editor "SANDBOX DIRECT METHOD" sandbox pool
  if rc \= 0 then return 5
  if pool.get("DIRECT") \= "native-direct" then return 6

  rc = -1
  address editor "SANDBOX ROUNDTRIP" sandbox pool
  if rc \= 0 then return 7
  if pool.get("RESULT") \= "native-input:native" then return 8

  pool = .standardaddresssandbox()
  rc = -1
  address editor
  "SANDBOX DIRECT METHOD" sandbox pool
  if rc \= 0 then return 9
  if pool.get("DIRECT") \= "native-direct" then return 10

  items[1] = "native-one"
  items[2] = "native-two"
  rc = -1
  address editor "EXPOSE ARRAY" expose items[]
  if rc \= 0 then return 11
  if items[1] \= "native-one" then return 12
  if items[2] \= "native-two-updated" then return 13
  if items[3] \= "native-three" then return 14

  return 0
