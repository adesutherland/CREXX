options levelb comments_dash
namespace data_List expose List
import ListIterator
import rxfnsb
/**
* A dynamic, ordered collection backed by a stringarray
*
* The List supports 1-based indexed access via the [] operator and grows
* automatically as elements are appended.
*
 * Internal representation:
 *   val.1 .. val.n   - elements (stem tails)
 *   card             - number of elements in the list
 *
 * This class is optimized for:
 *   - fast append (O(1))
 *   - fast indexed access (O(1))
 *
 * Insertions/removals in the middle require shifting elements (O(n)).
 */
 List: class
 card = .int
 val = .string[]
 
 /**
 * Factory method, returns a List object, which initially is empty
 */
  *: factory
    card = 0
    return
    
 /**
 * Initializes an empty list.
 */
  init: method = .int
    card = 0
    return 1
    
    /**
    * Appends an element to the end of the list.
    *
    * @param item  The element to add.
    */
  add: method = .int
    arg item = .string
    card = card + 1
    val.card = item
    return 1
    
    
    /**
    * Returns the element at the specified index.
    *
    * @param index  1-based index of the element.
    * @return       The element at the given index, or 0 if out of bounds. TODO exception
    */
  get: method = .string
    arg index = .int
    if index < 1 then return 0 -- TODO exception
    if index > card then return 0 -- TODO exception
    return val.index
      
      
      /**
      * Replaces the element at the specified index.
      *
      * @param index  1-based index of the element.
      * @param item   The new value.
      */
  set: method = .int
    arg index = .int, item = .string
    if index >= 1 then if index <= card then
      val.index = item
      return 1
      
      /**
      * Inserts an element at the specified index.
      *
      * Elements at and after the index are shifted to the right.
      *
      * @param index  1-based index where the element will be inserted.
      * @param item   The element to insert.
      */
  insert: method = .int
    arg index = .int, item = .string
    
    if index < 1 then index = 1
    if index > card + 1 then index = card + 1
    
    do i = card to index by -1
      h=i+1
      val.h = val.i
    end
    
    val.index = item
    card = card + 1
    return 1
    
    
    /**
    * Removes and returns the element at the specified index.
    *
    * Elements after the index are shifted left.
    *
    * @param index  1-based index of the element to remove.
    * @return       The removed element, or 0 if out of bounds.
    */
  remove: method = .string
     arg index = .int
    
    if index < 1 | index > card then
      return 0 -- TODO raise exception
      
      item = val.index
      
      do i = index to card - 1
	h=i+1
	val.i = val.h
      end
      
      val.card = ''
      card = card - 1
      
      return item
      
      
      /**
      * Returns the number of elements in the list.
      *
      * @return The size of the list.
      */
  size: method = .int
    return card
    
    
    /**
    * Tests whether the list is empty.
    *
    * @return 1 if empty, 0 otherwise.
    */
  isEmpty: method = .int
    return card = 0
    
    /**
    * Returns an iterator over this list.
    *
    * @return A ListIterator.
    */
  -- iterator: method = .ListIterator
  --   return .ListIterator(this)
    
    /**
    * Removes all elements from the list.
    */
  clear: method = .void
    do i = 1 to card
      val.i = ''
    end
    card = 0
    return
    
    
