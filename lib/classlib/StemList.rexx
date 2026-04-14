options levelb comments_dash
namespace data_List expose StemList
import rxfnsb

/**
 * A dynamic, ordered collection backed by a stem object.
 *
 * The List supports 1-based indexed access via the [] operator.
 *
 * Internal representation:
 *   val_   - backing stem object
 *   size_  - number of elements in the list
 *
 * Elements are stored in the stem under keys "1", "2", ..., "size_".
 */
StemList: class
size_ = .int
val_  = .stem

/**
* Factory method, returns an empty StemList object.
*/
  *: factory
    size_ = 0
    val_ = .stem()
    return
    
    /**
    * Initializes an empty list.
    */
  init: method = .int
    size_ = 0
    val_ = .stem()
    return 1
    
    /**
    * Appends an element to the end of the list.
    *
    * @param item  The element to add.
    * @return      1 on success
    */
  add: method = .int
    arg item = .string
    size_ = size_ + 1
    val_.size_ = item
    return 1
    
    /**
    * Returns the element at the specified index.
    *
    * @param index  1-based index of the element.
    * @return       The element at the given index, or 0 if out of bounds.
    */
  get: method = .string
    arg index = .int
    
    if index < 1 then return 0
    if index > size_ then return 0
    
    return val_.get(index)
    
    /**
    * Replaces the element at the specified index.
    *
    * @param index  1-based index of the element.
    * @param item   The new value.
    * @return       1 on success, 0 if out of bounds
    */
  set: method = .int
    arg index = .int, item = .string
    
    if index < 1 then return 0
    if index > size_ then return 0
    
    val_.index = item
    return 1
    
    /**
    * Inserts an element at the specified index.
    *
    * Elements at and after the index are shifted to the right.
    *
    * @param index  1-based index where the element will be inserted.
    * @param item   The element to insert.
    * @return       1 on success
    */
  insert: method = .int
    arg index = .int, item = .string
    
    if index < 1 then index = 1
    if index > size_ + 1 then index = size_ + 1
    
    do i = size_ to index by -1
      h = i + 1
      val_.h = val_.i
    end
    
    val_.index = item
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
    
    if index < 1 then return 0
    if index > size_ then return 0
    
    item = val_.get(index)
    
    do i = index to size_ - 1
      h = i + 1
      val_.i =  val_.h
    end
    
    /* No delete in stem yet; blank out old last slot */
    val_.size_ = ""
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
    * Removes all elements from the list.
    *
    * @return 1 on success
    */
  clear: method = .int
    do i = 1 to size_
      val_.i = ""
    end
    size_ = 0
    return 1

    /*
    * method toStem returns the stem object
    * from this StemList
    * @return .stem val_
    */
  toStem: method = .stem    
    return val_    
    
    /**
    * Returns an iterator over this list.
    *
    * @return a StemListIterator object.
    */
    -- iterator: method = .object
    --   return .ListIterator(this)
    iterator: method = .StemListIterator
      return .StemListIterator(val_)
    
