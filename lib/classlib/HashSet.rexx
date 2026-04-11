options levelb
namespace data_HashSet expose HashSet
import treemap

HashSet: class
val = .int

  *: factory
    arg expected = 1024
    val = stemcreate(expected, 'hashset')
    return

  add: method = .int
    arg key = .string
    return stemput(val, key, key)

  contains: method = .int
    arg key = .string
    return stemcontainskey(val, key)

  remove: method = .int
    arg key = .string
    return stemremove(val, key)

  size: method = .int
    return stemsize(val)

  toString: method = .string
    keys = .string[]
    vals = .string[]
    n = stemiterate(val, keys, vals)

    if n = 0 then return '{}'

    s = '{'
    loop i = 1 to n
      if i > 1 then s = s || ', '
      s = s || keys[i]
    end
    return s || '}'

  free: method = .int
    rc = stemfree(val)
    val = 0
    return rc

