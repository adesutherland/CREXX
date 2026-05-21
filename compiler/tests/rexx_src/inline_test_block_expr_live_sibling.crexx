options levelb

main: procedure = .int
  holder = .Holder("seed")
  say holder.summary()
  return 0

Holder: class
  name = .string
  left = .string
  right = .string
  child = .Child

  *: factory
    arg label = .string
    name = label
    left = label || ":left"
    right = label || ":right"
    child = .Child(label || ":child")
    return

  summary: method = .string
    return name || "|" || left || "|" || child.getName() || "|" || right

Child: class
  name = .string

  *: factory
    arg label = .string
    name = label
    return

  getName: method = .string
    return name
