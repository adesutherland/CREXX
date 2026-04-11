options levelb
namespace data_TreeMap expose TreeMap
import treemap
import TreeMapIterator

/** 
 * TreeMap: a Red-Black tree based map that stores key-value
 * pairs in the natural order of the keys, backed by the 
 * treemap.c plugin.
 * In this version all keys and values are strings.
 * 
 * @author René Vincent Jansen
 * @author Peter Jacob
 */

TreeMap: class
val = .int

/** method factory creates an instance of the TreeMap
 * class and returns it. Constructs a new, empty tree map,
 *  using the natural ordering of its keys.
 */
  *: factory
    val = tmcreate('val')
    return
   /**
    * Associates the specified value with the specified key in this map.
    * @parm .string key
    * @parm .string value
    * @return .int
    */  
  put: method = .int
    arg key = .string, value = .string
    return tmput(val,key,value)

    /**
     * Returns the value to which the specified key is mapped, 
     * or nothing if this map contains no mapping for the key.
     * @parm .string key
     * @return .string the found value
     */
  get: method = .string
    arg key = .string
    return tmget(val,key)

    /**
     * Returns the number of key-value mappings in this TreeMap.
     * return .int size
     */
  size: method = .int
    return tmsize(val)  

    /**
     * method remove removes a mapping from this TreeMap
     * @parm .string key
     * @return .int 0 for success, 4 for failure
     */
  remove: method = .int
    arg key = .string
    return tmremove(val,key)
    
    /**
     * method containsKey returns 1 (true) if this map contains
     * a mapping for the specified key.
     * @parm .string key
     * @return .int 
     */
  containsKey: method = .int
    arg key = .string
    return tmcontainsKey(val,key)

    /**
     * method firstKey returns the first (lowest) key 
     * currently in this map.
     * @parm .string key
     * @return .int 
     */
  firstKey: method = .string
    return tmfirstkey(val)

    /**
     * method lastKey returns the last (highest) key 
     * currently in this map.
     * @parm .string key
     * @return .int 
     */
  lastKey: method = .string
    return tmlastkey(val)

    /**
     * method keystem returns the keys in this TreeMap 
     * as a rexx stem
     */
  keyStem: method = .string[]
    list = .string[]
    n = tmkeys(val, list)
    return list

    /**
     * method iterator returns a TreeMapIterator 
     * which iterates over the keys currently in this map.
     * @return .TreeMapIterator 
     */
  iterator: method = .TreeMapIterator
    return .TreeMapIterator(val)

    /**
    * method toString() returns the content of the TreeMap
    * as a string.
    * @return .string
    */
  toString: method = .string
    return tmtostring(val)
   
