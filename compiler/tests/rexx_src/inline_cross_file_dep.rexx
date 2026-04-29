options levelb
namespace inline_cross_file_dep expose inc classify scoped nested sumTo countUntil buildArray renderArray identityBox refBump optAdd box

inc: procedure = .int
  arg value = .int
  return value + 1

classify: procedure = .int
  arg value = .int
  if value < 0 then return -1
  if value > 0 then return 1
  return 0

scoped: procedure = .int
  arg value = .int
  if value > 0 then do
    tmp = .int
    tmp = value * 10
    return tmp
  end
  return value

nested: procedure = .int
  arg value = .int
  staged = inc(value)
  return staged * 2

sumTo: procedure = .int
  arg limit = .int
  total = 0
  do i = 1 to limit
    total = total + i
  end
  return total

countUntil: procedure = .int
  arg limit = .int
  total = 0
  do while total < limit
    total = total + 1
  end
  return total

buildArray: procedure = .string[]
  arg left = .string, right = .string
  temp = .string[]
  temp[1] = left
  temp[2] = right
  return temp

renderArray: procedure = .string
  arg value = .string[]
  return value[1] || ":" || value[2]

identityBox: procedure = .inline_cross_file_dep..box
  arg value = .inline_cross_file_dep..box
  return value

refBump: procedure = .int
  arg expose value = .int
  value = value + 1
  return value

optAdd: procedure = .int
  arg ?a = 10, ?b = 5
  return a + b

box: class
  name = .string

  *: factory
    arg initial = .string
    name = initial
    return

  getName: method = .string
    return name
