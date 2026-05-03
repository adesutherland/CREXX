options levelb
namespace inline_local_member_scalar

main: procedure = .int
  sample = .Box("alpha")
  say sample.get()
  call sample.set("beta")
  say sample.get()
  other = .Box("gamma")
  say other.get()
  return 0

Box: class
  value = .string

  *: factory
    arg initial = .string
    value = initial
    return

  get: method = .string
    return value

  set: method = .void
    arg next = .string
    value = next
    return
