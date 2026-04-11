options levelb
namespace data_HashSet expose HashSetIterator
import treemap

HashSetIterator: class
token = .int
closed = .int

  *: factory
    arg setToken = .int
    token = stemitercreate(setToken)
    closed = 0
    return

  hasNext: method = .int
    if closed then return 0
    return stemiterhasnext(token)

  next: method = .string
    if closed then return ''
    return stemiternext(token)

  close: method = .int
    if \closed then do
      call stemiterfree(token)
      token = 0
      closed = 1
    end
    return 0
