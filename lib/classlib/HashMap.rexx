options levelb
namespace data_HashMap expose HashMap
import treemap

/** 
 * class HashMap offers a Map implementation backed by a hashed stem
 * with dynamic buckets. The storage and representations is unordered.
 * 
 * @author René Vincent Jansen
 * @author Peter Jacob
 */
HashMap: class
val = .int

/** 
 * the class factory returns an instance of the HashMap class
 * the expected cardinality can be indicated by a parameter
 * (which defaults to 1024)
 * @param .string expected
 */
  *: factory
    arg expected = 1024
    val = stemcreate(expected, 'hashmap')
    return

    /**
    * Associates the specified value with the specified key in this map.
    * @parm .string key
    * @parm .string value
    * @return .int
    */  
  put: method = .int
    arg key = .string, value = .string
    return stemput(val, key, value)
    
    /**
    * Returns the value to which the specified key is mapped, 
    * or nothing if this map contains no mapping for the key.
    *  @parm .string key
    * @return .string the found value
    */
  get: method = .string
    arg key = .string
    return stemget(val, key)

    /**
    * method containsKey returns 1 (true) if this map contains
    * a mapping for the specified key.
    * @parm .string key
    * @return .int 
    */
  containsKey: method = .int
    arg key = .string
    return stemcontainskey(val, key)

    /**
     * method remove removea a mapping from this HashMap
     * @parm .string key
     * @return .int 0 for success, 4 for failure
     */
  remove: method = .int
    arg key = .string
    return stemremove(val, key)

    /**
    * Returns the number of key-value mappings in this HashMap.
    * return .int size
    */
  size: method = .int
    return stemsize(val)

    /**
     * method keystem returns the keys in this HashMap 
     * as a Rexx stem
     */
  keyStem: method = .string[]
    list = .string[]
    vals = .string[]
    n = stemiterate(val, list, vals)
    return list

    /**
    * method iterator returns a HashMapIterator 
    * which iterates over the keys currently in this map.
    * @return .HashMapIterator 
    */
  iterator: method = .HashMapIterator
    return .HashMapIterator(val)

    /**
    * method toString() returns the content of the TreeMap
    * as a string.
    * @return .string
    */
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
    
