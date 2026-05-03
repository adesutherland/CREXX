options levelb
namespace set6_counter expose CounterIterator

CounterIterator: class
current = .int
limit = .int

  *: factory
    arg max = .int
    current = 1
    limit = max
    return

  hasNext: method = .int
    return current <= limit

  next: method = .int
    value = current
    current = current + 1
    return value
