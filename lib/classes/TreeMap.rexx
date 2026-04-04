options levelb

namespace data_TreeMap expose TreeMap
import treemap

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
    
    
