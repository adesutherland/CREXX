options levelb 
namespace data_Stack expose Stack

/**
 * The Stack class represents a last-in-first-out (LIFO) stack of objects.
 */
Stack: class
val   = .string[]
sp    = .int

/**
 * The factory method returns a Stack object, which initially is empty
 */
  *: factory
    sp = 0
    return

    /** 
     * method push
     * Pushes an item onto the top of this stack.
     * @parm .string 
     * @return .int (true)
     */ 
  push: method = .int
    arg item = .string
    sp = sp + 1
    val[sp] = item
    return 1

    /**
     * method size returns the number of elemenst on the stack
     * @return .int
     */
  size: method = .int
    return sp

    /**
     * method isEmpty returns 1 (true) when the stack is empty
     * and 0 (false) when it is not
     * @return .int
     */
  isEmpty: method = .int
    if sp = 0 then return 1
    return 0

    /**
    * method empty returns 1 (true) when the stack is empty
    * and 0 (false) when it is not
    * it is a synonym of isEmpty
    * @return .int
    */
  empty: method = .int
    if sp = 0 then return 1
    return 0

    /**
     * Removes the object at the top of this stack 
     * and returns that object as the value of this function.
     * @return .string item
     */
  pop: method = .string
    if sp = 0 then return 0
    item = val[sp]
    sp = sp - 1
    return item

    /**
     * Looks at the object at the top of this stack 
     * without removing it from the stack.
     * @return .string item
     */
  peek: method = .string
    if sp = 0 then return 0
    item = val[sp]
    return item

