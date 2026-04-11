options levelb
namespace data_HashMap expose HashMapIterator
import treemap

HashMapIterator: class
token = .int
closed = .int

  *: factory
    arg mapToken = .int
    token = stemitercreate(mapToken)
    closed = 0
    return

  hasNext: method = .int
    if closed then return 0
    return stemiterhasnext(token)

  next: method = .string
    if closed then return ''
    return stemiternext(token)

  nextKey: method = .string
    if closed then return ''
    return stemiternext(token)

  nextValue: method = .string
    if closed then return ''
    return stemitervalue(token)

  close: method = .int
    if \closed then do
      call stemiterfree(token)
      token = 0
      closed = 1
    end
    return 0
