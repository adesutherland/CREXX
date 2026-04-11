options levelb
namespace data_HashSet expose HashSet
import treemap

/**
* This class implements a Set interface, backed by a stem instance. 
* It makes no guarantees as to the iteration order of the set; 
* in particular, it does not guarantee that the order will remain constant over time.
* 
* @author René Vincent Jansen
* @author Peter Jacob
*/
HashSet: class
val = .int

/** The factory method returns an instance of HashSet.
 * The expected cardinality can be indicated by a parameter
 * (which defaults to 1024)
 * @param .string expected
 * @return .HashSet
 */
  *: factory
    arg expected = 1024
    val = stemcreate(expected, 'hashset')
    return

    /** method add adds the specified element to this set 
     * if it is not already present. Otherwise it will leave
     * the set unchanged.
     * @parm .string element
     */
  add: method = .int
    arg key = .string
    return stemput(val, key, key)

    /**
    * method contains returns 1 (true) if this set contains
    * the specified element.
    * @parm .string element
    * @return .int 
     */
  contains: method = .int
    arg key = .string
    return stemcontainskey(val, key)

    /**
     * method remove removes an element from this HashSet
     * @parm .string key
     * @return .int 0 for success, 4 for failure
     */
  remove: method = .int
    arg key = .string
    return stemremove(val, key)

    /**
     * Returns the number of elements in this HashSet.
     * return .int size
     */
  size: method = .int
    return stemsize(val)

    /**
    * method keystem returns the items in this HashSet 
    * as a rexx stem
    */
  toStem: method = .string[]
    keys = .string[]
    vals = .string[]
    n = stemiterate(val, keys, vals)
    return keys

    /**
    * method iterator returns a HashSetIterator 
    * which iterates over the keys currently in this map.
    * @return .HashSetIterator 
    */
  iterator: method = .HashSetIterator
    return .HashSetIterator(val)

    /**
    * method toString() returns the content of the HashSet
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
      s = s || keys[i]
    end
    return s || '}'

    /**
     * method free returns the memory of this HashSet
     * to the heap.
     * @return int
     */
  free: method = .int
    rc = stemfree(val)
    val = 0
    return rc

