options levelb
namespace class3_lib expose Counter

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

  value2: method = .int
    return val * 2

  set: method = .void
    arg new_val = .int
    val = new_val
    return
