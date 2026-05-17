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
namespace data_HashMap expose HashMapIterator
import treemap

/**
 * Class HashMapIterator returns an iterator
 * for a HashMap object. It is short for
 * HashMap.getKeySet.iterator(). It is a 'live'
 * iterator, as opposed to a 'snapshot' one.
 * This implies that undefined behaviour is
 * possible (and expected) when the underlying 
 * HashMap is modified during its execution.
 */

HashMapIterator: class
token = .int
closed = .int

  *: factory
    arg mapToken = .int
    token = stemitercreate(mapToken)
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
    
  nextKey: method = .string
    if closed then return ''
    return stemiternext(token)
    
  nextValue: method = .string
    if closed then return ''
    return stemitervalue(token)

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
