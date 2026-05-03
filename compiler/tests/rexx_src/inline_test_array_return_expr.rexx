options levelb

main: procedure = .int
  say "Starting array return expr inline test..."
  say renderArray(buildArray("red", "blue"))
  say renderArray(buildArray("green", "gold"))
  say "Array return expr inline test finished."
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
