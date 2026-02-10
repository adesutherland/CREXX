options levelb

c = .Counter()
c.inc()
c.inc()
say "Count: " || c.value()
c.set(10)
say "Count: " || c.value()

return 0

Counter: class
  val = .int with register.1

  *: factory
    val = 0
    return

  inc: method
    val = val + 1
    return

  value: method = .int
    return val

  set: method = .void
    arg new_val = .int
    val = new_val
    return
