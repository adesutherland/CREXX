/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

options levelb
namespace data_HashSet expose HashSet
import treemap

/**
 * This class implements a Set interface, backed by a stem instance. 
 * It makes no guarantees as to the iteration order of the set; 
 * in particular, it does not guarantee that the order will remain constant over time.
 * 
 * @author René Vincent Jansen
 * @author Peter Jacob
*/
HashSet: class
val = .int

/** The factory method returns an instance of HashSet.
 * The expected cardinality can be indicated by a parameter
 * (which defaults to 1024)
 * @param .string expected
 * @return .HashSet
 */
  *: factory
    arg expected = 1024
    val = stemcreate(expected, 'hashset')
    return

    /** method add adds the specified element to this set 
     * if it is not already present. Otherwise it will leave
     * the set unchanged.
     * @parm .string element
     */
  add: method = .int
    arg key = .string
    return stemput(val, key, key)

    /**
     * method fromStem adds elements to this HashSet instance
     * from a .string[], and adds them without order.
     * @parm .string[] items
     * @result .int added
     */
  fromArray: method = .int
    arg items = .string[]
    added = 0
    loop i = 1 to items.0
      rc = add(items.i)
      if rc = 0 then added = added + 1
    end
    return added

    /**
     * method contains returns 1 (true) if this set contains
     * the specified element.
     * @parm .string element
     * @return .int 
     */
  contains: method = .int
    arg key = .string
    return stemcontainskey(val, key)
    
    /**
     * method remove removes an element from this HashSet
     * @parm .string key
     * @return .int 0 for success, 4 for failure
     */
  remove: method = .int
    arg key = .string
    return stemremove(val, key)
    
    /**
     * Returns the number of elements in this HashSet.
     * return .int size
     */
  size: method = .int
    return stemsize(val)

    /**
     * method toArray returns the items in this HashSet 
     * as a rexx .string[]
     */
  toArray: method = .string[]
    keys = .string[]
    vals = .string[]
    n = stemiterate(val, keys, vals)
    return keys
    
    /**
     * method iterator returns a HashSetIterator 
     * which iterates over the keys currently in this map.
     * @return .HashSetIterator 
     */
  iterator: method = .HashSetIterator
    return .HashSetIterator(val)
    
    /**
     * method toString() returns the content of the HashSet
     * as a string.
     * @return .string
     */
  toString: method = .string
    keys = .string[]
    vals = .string[]
    n = stemiterate(val, keys, vals)
    
    if n = 0 then return '{}'
    
    s = '{'
    loop i = 1 to n
      if i > 1 then s = s || ', '
      s = s || keys[i]
    end
    return s || '}'
    
    /**
     * method free returns the memory of this HashSet
     * to the heap.
     * @return int
     */
  free: method = .int
    rc = stemfree(val)
    val = 0
    return rc

