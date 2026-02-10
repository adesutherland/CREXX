options levelb
import class3_lib

c = .Counter()
c.inc()
c.inc()
say "Count: " || c.value()
say "Count2: " || c.value2()
c.set(10)
say "Count: " || c.value()

return 0
