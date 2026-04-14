options levelb
namespace data_TreeSet expose TreeSet
import treemap
import TreeSetIterator

/**
* class TreeSet implements a Set, backed by a treemap instance. 
* It stores the elements, which are strings, in their natural order.
* 
* @author René Vincent Jansen
* @author Peter Jacob
*/

TreeSet: class
val = .int

/** 
 * the class factory returns an instance of the TreeSet class
 */
  *: factory
    val = tmcreate('set')
    return

    /** method add adds the specified element to this set 
     * if it is not already present. Otherwise it will leave
     * the set unchanged. It will be added according to its
     * natural order..
     * @parm .string element
     * @return .int
     */
  add: method = .int
    arg key = .string
    return tmput(val, key, key)

/**
 * method fromArray adds elements to this TreeSet instance
 * from an array, and deduplicates and orders them.
 * @parm .string items
 * @result .int added
 */
  fromArray: method = .int
    arg items = .string[]
    added = 0
    loop i = 1 to items.0
      rc = add(items.i)
      if rc = 0 then added = added + 1
    end
    return added
    
    /**
    * method contains returns 1 (true) if this set contains
    * the specified element.
    * @parm .string element
    * @return .int 
    */
  contains: method = .int
    arg key = .string
    return tmcontainsKey(val, key)

    /**
    * method remove removes an element from this TreeSet
    * @parm .string key
    * @return .int 0 for success, 4 for failure
    */
  remove: method = .int
    arg key = .string
    return tmremove(val, key)

    /**
     * Returns the number of elements in this TreeSet.
     * return .int size
     */
  size: method = .int
    return tmsize(val)

    /**
     * method first returns the first (lowest) element 
     * currently in this set.
     * @return .int 
     */
  first: method = .string
    return tmfirstkey(val)
    
    /**
     * method last returns the last (highest) element 
     * currently in this set.
     * @return .int 
     */
  last: method = .string
    return tmlastkey(val)

    /**
    * method iterator returns a TreeSetIterator 
    * which iterates over the elements currently in this set.
    * @return .TreeMapIterator 
    */
  iterator: method = .TreeSetIterator
    return .TreeSetIterator(val)

    /**
    * method toArray returns the items in this TreeSet 
    * as a rexx array
    */
  toArray: method = .string[]
    list = .string[]
    n = tmkeys(val, list)
    return list

    /**
    * method keystem returns the elements in this TreeSet 
    * as a string
    */
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
