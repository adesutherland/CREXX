options levelb
namespace set6_counter expose Counter
import CounterIterator

Counter: class
limit = .int

  *: factory
    arg max = .int
    limit = max
    return

  iterator: method = .CounterIterator
    return .CounterIterator(limit)
