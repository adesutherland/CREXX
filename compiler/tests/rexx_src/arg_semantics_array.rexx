options levelb
import rxfnsb

main: procedure = .int
  arr = .string[]
  arr[1] = "red"
  arr[2] = "blue"

  say "array-read=" || readArray(arr)
  say "array-mut=" || mutateArrayLocal(arr)
  say "array-after=" || arr[1] || ":" || arr[2]
  say "array-temp=" || mutateArrayLocal(makeArray())
  call mutateArrayRef(arr)
  say "array-ref=" || arr[1] || ":" || arr[2]
  return 0

readArray: procedure = .string
  arg value = .string[]
  return value[1] || ":" || value[2]

mutateArrayLocal: procedure = .string
  arg value = .string[]
  value[1] = upper(value[1])
  value[2] = "local"
  return value[1] || ":" || value[2]

mutateArrayRef: procedure = .void
  arg expose value = .string[]
  value[1] = "ref"
  value[2] = upper(value[2])
  return

makeArray: procedure = .string[]
  temp = .string[]
  temp[1] = "temp"
  temp[2] = "seed"
  return temp
