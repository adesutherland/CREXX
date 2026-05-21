options levelb
import rxfnsb

main: procedure = .int
  errors = 0
  direct = "client-input"
  stem = .string[]
  pool = .standardaddresssandbox()

  address editor "DIRECT" expose direct
  if rc <> 0 then errors = errors + 1
  if direct <> "client-updated" then errors = errors + 2

  scalar = "scalar-before"
  address editor "SCALAR_COMPAT" expose scalar
  if rc <> 0 then errors = errors + 256
  if scalar <> "scalar-from-tail" then errors = errors + 512

  address editor "STEM" expose stem[]
  if rc <> 0 then errors = errors + 4
  if stem[1] <> "stem-one" then errors = errors + 8
  if stem.0 <> 1 then errors = errors + 16

  call pool.set("SANDBOX_IN", "sandpit-input")
  address editor "SANDBOX" sandbox pool
  if rc <> 0 then errors = errors + 32
  if pool.get("SANDBOX_IN") <> "sandpit-input-read" then errors = errors + 64
  if pool.get("SANDBOX_OUT") <> "sandpit-output" then errors = errors + 128

  return errors
