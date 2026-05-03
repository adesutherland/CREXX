options levelb

main: procedure = .int
  data = .binary
  data = relayBinary(makeBinary())
  say "ok"
  return 0

relayBinary: procedure = .binary
  arg data = .binary
  return data

makeBinary: procedure = .binary
  data = .binary
  data = "abc"
  return data
