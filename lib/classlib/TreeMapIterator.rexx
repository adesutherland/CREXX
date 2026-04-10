options levelb
namespace data_TreeMap expose TreeMapIterator
import treemap

TreeMapIterator: class
token = .int
closed = .int

  *: factory
    arg mapToken = .int
    token = tmitercreate(mapToken)
    closed = 0
    return

  hasNext: method = .int
    if closed then return 0
    return tmiterhasnext(token)

  next: method = .string
    if closed then return ''
    return tmiternext(token)

  close: method = .int
    if \closed then do
      call tmiterfree(token)
      closed = 1
      token = 0
    end
    return 0
