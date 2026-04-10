options levelb
namespace data_TreeSet expose TreeSetIterator
import treemap


TreeSetIterator: class
token = .int

  *: factory
    arg mapToken = .int
    token = tmitercreate(mapToken)
    return

  hasNext: method = .int
    return tmiterhasnext(token)

  next: method = .string
    return tmiternext(token)

  close: method
    call tmiterfree(token)
    return
