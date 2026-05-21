options levelb

main: procedure = .int
  say "Starting array multi return assign inline test..."
  left = selectArray(-1)
  say renderArray(left)
  mid = selectArray(0)
  say renderArray(mid)
  right = selectArray(1)
  say renderArray(right)
  say "Array multi return assign inline test finished."
  return 0

selectArray: procedure = .string[]
  arg flag = .int
  neg = .string[]
  zero = .string[]
  pos = .string[]
  neg[1] = "neg"
  neg[2] = "path"
  zero[1] = "zero"
  zero[2] = "path"
  pos[1] = "pos"
  pos[2] = "path"
  if flag < 0 then return neg
  if flag > 0 then return pos
  return zero

renderArray: procedure = .string
  arg value = .string[]
  return value[1] || ":" || value[2]
