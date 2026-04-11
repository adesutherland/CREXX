options levelb
namespace data_HashMap expose HashMap
import treemap

HashMap: class
val = .int

  *: factory
    arg expected = 1024
    val = stemcreate(expected, 'hashmap')
    return

  put: method = .int
    arg key = .string, value = .string
    return stemput(val, key, value)

  get: method = .string
    arg key = .string
    return stemget(val, key)

  containsKey: method = .int
    arg key = .string
    return stemcontainskey(val, key)

  remove: method = .int
    arg key = .string
    return stemremove(val, key)

  size: method = .int
    return stemsize(val)

  keys: method = .string[]
    list = .string[]
    vals = .string[]
    n = stemiterate(val, list, vals)
    return list

  toString: method = .string
    keys = .string[]
    vals = .string[]
    n = stemiterate(val, keys, vals)

    if n = 0 then return '{}'

    s = '{'
    loop i = 1 to n
      if i > 1 then s = s || ', '
      s = s || keys[i] || '=' || vals[i]
    end
    return s || '}'

  free: method = .int
    rc = stemfree(val)
    val = 0
    return rc
    
