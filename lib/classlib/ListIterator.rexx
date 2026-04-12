options levelb comments_dash
namespace data_List expose ListIterator
import List

/**
 * Iterator for the List class.
 *
 * Supports forward traversal of a List.
 *
 * The iterator maintains a cursor positioned between elements:
 *
 *   index_ = 0       → before first element
 *   index_ = n       → after returning element n
 *
 * Typical usage:
 *
 *   it = list.iterator()
 *   do while it.hasNext()
 *     say it.next()
 *   end
 */
ListIterator: class

list_  = .List
index_ = .int


/**
 * Factory method.
 *
 * @param list  The List to iterate over.
 */
*: factory
  arg list = .List
  -- arg s = .stem
  list_  = list
  index_ = 0
  return


/**
 * Returns 1 if more elements are available.
 *
 * @return 1 if there is a next element, 0 otherwise.
 */
hasNext: method = .int
  return index_ < list_.size()


/**
 * Returns the next element in the iteration.
 *
 * Advances the cursor.
 *
 * @return The next element, or 0 if none available.
 */
next: method = .string
  if index_ >= list_.size() then
    return 0  -- TODO exception

  index_ = index_ + 1
  return list_.index_


/**
 * Returns the current index (1-based).
 *
 * @return Current position in the list.
 */
index: method = .int
  return index_


/**
 * Resets the iterator to the start.
 */
reset: method = .void
  index_ = 0
  return
