options levelb
namespace data_TreeSet expose TreeSetIterator
import treemap

/**
 * Class TreeSetIterator returns an iterator
 * for a TreeSet object.
 * It is a 'live' iterator, as opposed to a 'snapshot' one.
 * This implies that undefined behaviour is
 * possible (and expected) when the underlying 
 * HashSet is modified during its execution.
 */

TreeSetIterator: class
token = .int

/**
* factory method returns an instance of this class
*/
  *: factory
    arg mapToken = .int
    token = tmitercreate(mapToken)
    return

    /** method hasNext() returns 1 (true)
    * as long as there are more items
    * to be returned by this iterator
    */
  hasNext: method = .int
    return tmiterhasnext(token)

    /**
    * method next() returns the next item
    * available from this iterator
    */
  next: method = .string
    return tmiternext(token)

    /**
    * method close() closes this 
    * iterator. After this, hasNext()
    * returns 0
    */
  close: method
    call tmiterfree(token)
    return
