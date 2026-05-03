options levelb

main: procedure = .int
  say "Starting array return assign inline test..."
  values = buildArray("red", "blue")
  say values[1] || ":" || values[2]
  more = buildArray("green", "gold")
  say renderArray(more)
  say "Array return assign inline test finished."
  return 0

buildArray: procedure = .string[]
  arg left = .string, right = .string
  temp = .string[]
  temp[1] = left
  temp[2] = right
  return temp

renderArray: procedure = .string
  arg value = .string[]
  return value[1] || ":" || value[2]
