options levelb comments_dash
namespace data_List expose ArrayList


/**
* A dynamic, ordered collection backed by a stringarray
*
* The List supports 1-based indexed access via the [] operator and grows
* automatically as elements are appended.
*
 * Internal representation:
 *   val.1 .. val.n   - elements (stem tails)
 *   size_             - number of elements in the list
 *
 * This class is optimized for:
 *   - fast append (O(1))
 *   - fast indexed access (O(1))
 *
 * Insertions/removals in the middle require shifting elements (O(n)).
 */
ArrayList: class
 size_ = .int
 val = .string[]
 
 /**
 * Factory method, returns a List object, which initially is empty
 */
  *: factory
    size_ = 0
    return
    
 /**
 * Initializes an empty list.
 */
  init: method = .int
    size_ = 0
    return 1
    
    /**
    * Appends an element to the end of the list.
    *
    * @param item  The element to add.
    */
  add: method = .int
    arg item = .string
    size_ = size_ + 1
    val.size_ = item
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
    if index > size_ then return 0 -- TODO exception
    return val.index
      
      
      /**
      * Replaces the element at the specified index.
      *
      * @param index  1-based index of the element.
      * @param item   The new value.
      */
  set: method = .int
    arg index = .int, item = .string
    if index >= 1 then if index <= size_ then
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
    if index > size_ + 1 then index = size_ + 1
    
    do i = size_ to index by -1
      h=i+1
      val.h = val.i
    end
    
    val.index = item
    size_ = size_ + 1
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
    
    if index < 1 | index > size_ then
      return 0 -- TODO raise exception
      
      item = val.index
      
      do i = index to size_ - 1
	h=i+1
	val.i = val.h
      end
      
      val.size_ = ''
      size_ = size_ - 1
      
      return item
      
      
      /**
      * Returns the number of elements in the list.
      *
      * @return The size of the list.
      */
  size: method = .int
    return size_
    
    
    /**
    * Tests whether the list is empty.
    *
    * @return 1 if empty, 0 otherwise.
    */
  isEmpty: method = .int
    return size_ = 0
    
    /**
    * Returns an iterator over this list.
    *
    * @return A ListIterator.
    */
  iterator: method = .string[]
    return val
    
    /**
    * Removes all elements from the list.
    */
  clear: method = .void
    do i = 1 to size_
      val.i = ''
    end
    size_ = 0
    return
    
    
