options levelb
import rxfnsb

main: procedure = .int
  i = 7
  say "int-read=" || inspectInt(i)
  say "int-mut=" || addTen(i)
  say "int-after=" || i
  call addOneRef(i)
  say "int-ref=" || i

  text = "mix"
  say "str-read=" || readText(text)
  say "str-mut=" || shoutText(text)
  say "str-after=" || text
  say "str-temp=" || shoutText("temp")
  return 0

inspectInt: procedure = .int
  arg value = .int
  return value + 1

addTen: procedure = .int
  arg value = .int
  value = value + 10
  return value

addOneRef: procedure = .void
  arg expose value = .int
  value = value + 1
  return

readText: procedure = .string
  arg text = .string
  return text || ":read"

shoutText: procedure = .string
  arg text = .string
  text = upper(text)
  return text || ":local"
