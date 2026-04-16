options levelb comments_dash
namespace data_LinkedList expose LinkedList
import llist
import rxfnsb

/**
 * LinkedList: a doubly linked list backed by the
 * llist.c plugin.
 *
 * In this version all elements are strings.
 *
 * @author René Vincent Jansen
 * @author Peter Jacob
 */

LinkedList: class
val = .int
nextid = .int

/** method factory creates an instance of the LinkedList
* class and returns it.
 */
  *: factory
    val	   = nextid
    nextid = nextid + 1
    return
    
  /**
   * method append adds a value at the end of the list.
   * @parm .string value
   * @return .int
   */
  append: method = .int
    arg value = .string
    return appendnode(val, value)
    
  /**
   * method prepend adds a value at the beginning of the list.
   * @parm .string value
   * @return .int
   */
  prepend: method = .int
    arg value = .string
    return prependnode(val, value)

  /**
   * method insertAfterCurrent inserts a value after the current node.
   * @parm .string value
   * @return .int
   */
  insertAfterCurrent: method = .int
    arg value = .string
    return insertnode(val, value, "AFTER")

  /**
   * method insertBeforeCurrent inserts a value before the current node.
   * @parm .string value
   * @return .int
   */
  insertBeforeCurrent: method = .int
    arg value = .string
    return insertnode(val, value, "BEFORE")

  /**
   * method remove removes the current node.
   * @return .int
   */
  remove: method = .int
    return removenode(val)

  /**
   * method current returns the current value.
   * It returns "" when the list is empty.
   * @return .string
   */
  current: method = .string
    s = currentnode(val)
    if left(s,12) = "$EMPTY-LLIST" then return ""
    return s

  /**
   * method next returns the next value and advances.
   * It returns "" when the end is reached.
   * @return .string
   */
  next: method = .string
    s = nextnode(val)
    if left(s,13) = "$END-OF-LLIST" then return ""
    return s

  /**
   * method previous returns the previous value and moves back.
   * It returns "" when the top is reached.
   * @return .string
   */
  previous: method = .string
    s = prevnode(val)
    if left(s,13) = "$TOP-OF-LLIST" then return ""
    return s

  /**
   * method currentAddress returns the address of the current node.
   * @return .int
   */
  currentAddress: method = .int
    return currentnodeaddr(val)

  /**
   * method setFirst positions current at the first node.
   * @return .int
   */
  setFirst: method = .int
    return setnode(val, "FIRST")

  /**
   * method setLast positions current at the last node.
   * @return .int
   */
  setLast: method = .int
    return setnode(val, "LAST")

  /**
   * method setPosition positions current by symbolic or numeric position.
   * Examples: "FIRST", "LAST", "3", "+1", "-1"
   * @parm .string pos
   * @return .int
   */
  setPosition: method = .int
    arg pos = .string
    return setnode(val, pos)

  /**
   * method setAddress sets the current node by address.
   * @parm .int addr
   * @return .int
   */
  setAddress: method = .int
    arg addr = .int
    return setnodeaddr(val, addr)

  /**
   * method clear frees all nodes in this list.
   * @return .int
   */
  clear: method = .int
    return freellist(val)

  /**
   * method size returns the number of elements in the list.
   * @return .int
   */
  -- size: method = .int
  --   a = valueArray()
  --   return a[0]

  /**
   * method isEmpty returns 1 if the list is empty, else 0.
   * @return .int
   */
  isEmpty: method = .int
    rc = setFirst()
    if current() = "" then return 1
    return 0

  /**
   * method valueArray returns the values in this LinkedList
   * as a rexx stem in traversal order.
   * @return .string[]
   */
  -- valueArray: method = .string[]
  --   list = .string[]
  --   i = 0
  --   rc = setFirst()
  --   item = current()
  --   loop while item <> ""
  --     i = i + 1
  --     list[i] = item
  --     item = next()
  --   end
  --   list[0] = i
  --   return list

  -- /**
  --  * method fromArray rebuilds this LinkedList from a rexx stem.
  --  * @parm .string[] list
  --  * @return .int
  --  */
  -- fromArray: method = .int
  --   arg list = .string[]
  --   rc = clear()
  --   loop i = 1 to list[0]
  --     rc = append(list[i])
  --   end
  --   return 0

  /**
   * method debug validates and reports on this list.
   * @return .int
   */
  -- debug: method = .int
  --   return debug(val)

  /**
   * method toString returns the content of the LinkedList
   * as a string.
   * @return .string
   */
  -- toString: method = .string
  --   a = valueArray()
  --   s = "{"
  --   loop i = 1 to a[0]
  --     if i > 1 then s = s || ", "
  --     s = s || a[i]
  --   end
  --   s = s || "}"
  --   return s

    
