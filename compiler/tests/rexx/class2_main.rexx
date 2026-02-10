options levelb
namespace class2_lib

c = .Counter()
c.inc()
c.inc()
say "Count: " || c.value()
c.set(10)
say "Count: " || c.value()

return 0

Counter: class
  *: factory
  inc: method
  value: method = .int
  set: method = .void
    arg new_val = .int
