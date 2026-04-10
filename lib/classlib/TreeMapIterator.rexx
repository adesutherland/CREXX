options levelb
namespace data_TreeMap expose TreeMapIterator
import treemap

/**
 * Class TreeMapIterator returns an iterator
 * for a TreeMap object. It is short for
 * TreeMap.getKeySet.iterator(). It is a 'live'
 * iterator, as opposed to a 'snapshot' one.
 * This implies that undefined behaviour is
 * possible (and expected) when the underlying 
 * TreeMap is modified during its execution.
 */

TreeMapIterator: class
token = .int
closed = .int

/**
 * factory method returns an instance of this class
 */
  *: factory
    arg mapToken = .int
    token = tmitercreate(mapToken)
    closed = 0
    return

    /** method hasNext() returns 1 (true)
     * as long as there are more items
     * to be returned by this iterator
     */
  hasNext: method = .int
    if closed then return 0
    return tmiterhasnext(token)

    /**
     * method next() returns the next item
     * available from this iterator
     */
  next: method = .string
    if closed then return ''
    return tmiternext(token)

    /**
     * method close() closes this 
     * iterator. After this, hasNext()
     * returns 0
     */
  close: method = .int
    if \closed then do
      call tmiterfree(token)
      closed = 1
      token = 0
    end
    return 0
