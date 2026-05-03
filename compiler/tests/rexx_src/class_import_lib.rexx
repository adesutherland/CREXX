options levelb

namespace class_import_lib expose Counter

Counter: class
  val = .int

  *: factory
    val = 0
    return

  inc: method = .void
    val = val + 1
    return

  value: method = .int
    return val
