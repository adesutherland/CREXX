options levelb 
namespace data_Stack expose Stack
import rxfnsb

Stack: class
val   = .string[]
sp    = .int

  *: factory
    sp = 0
    return
    
  push: method = .int
    arg item = .string
    sp = sp + 1
    val[sp] = item
    return 1
    
  size: method = .int
    return sp
    
  isEmpty: method = .int
    if sp = 0 then return 1
    return 0
    
  pop: method = .string
    if sp = 0 then return 0
    item = val[sp]
    sp = sp - 1
    return item

