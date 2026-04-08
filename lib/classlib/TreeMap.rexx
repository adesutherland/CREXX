options levelb
namespace data_TreeMap expose TreeMap
import treemap

/* 
 * TreeMap: a Red-Black tree based map that stores key-value
 * pairs in the natural order of the keys.
 * In this version the keys and values are strings.
 */

TreeMap: class
val = .int

  *: factory
    val = tmcreate('val')
    return
    
  put: method = .int
    arg key = .string, value = .string
    return tmput(val,key,value)
    
  get: method = .string
    arg key = .string
    return tmget(val,key)
    
  size: method = .int
    return tmsize(val)  
    
  containsKey: method = .int
    arg key = .string
    return tmcontainsKey(val,key)

  firstKey: method = .string
    return tmfirstkey(val)

  lastKey: method = .string
    return tmlastkey(val)

    
