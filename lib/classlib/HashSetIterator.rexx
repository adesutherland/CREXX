options levelb
namespace data_HashSet expose HashSetIterator
import treemap

/**
 * Class HashSetIterator returns an iterator
 * for a HashSet object.
 * It is a 'live' iterator, as opposed to a 'snapshot' one.
 * This implies that undefined behaviour is
 * possible (and expected) when the underlying 
 * HashSet is modified during its execution.
 */
HashSetIterator: class
token = .int
closed = .int

/**
* factory method returns an instance of this class
*/
  *: factory
    arg setToken = .int
    token = stemitercreate(setToken)
    closed = 0
    return

    /** method hasNext() returns 1 (true)
    * as long as there are more items
    * to be returned by this iterator
    */
  hasNext: method = .int
    if closed then return 0
    return stemiterhasnext(token)

    /**
    * method next() returns the next item
    * available from this iterator
    */
  next: method = .string
    if closed then return ''
    return stemiternext(token)

    /**
    * method close() closes this 
    * iterator. After this, hasNext()
    * returns 0
    */
  close: method = .int
    if \closed then do
      call stemiterfree(token)
      token = 0
      closed = 1
    end
    return 0
