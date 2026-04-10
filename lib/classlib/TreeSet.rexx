options levelb
namespace data_TreeSet expose TreeSet
import treemap
import TreeSetIterator

TreeSet: class
val = .int

  *: factory
    val = tmcreate('set')
    return

  add: method = .int
    arg key = .string
    return tmput(val, key, key)

  contains: method = .int
    arg key = .string
    return tmcontainsKey(val, key)

  remove: method = .int
    arg key = .string
    return tmremove(val, key)

  size: method = .int
    return tmsize(val)

  first: method = .string
    return tmfirstkey(val)

  last: method = .string
    return tmlastkey(val)

  iterator: method = .TreeSetIterator
    return .TreeSetIterator(val)

  toString: method = .string
    if size() = 0 then return '{}'

    it = iterator()
    result = '{'
    first = 1

    loop while it.hasNext()
      key = it.next()
      if first then first = 0
      else result = result || ', '
      result = result || key
    end

    call it.close()
    return result || '}'
