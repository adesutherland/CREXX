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

options levelb comments_dash
namespace data_List expose ArrayListIterator

/**
 * Iterator for the ArrayList class.
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
ArrayListIterator: class

list_  = .string[]
index_ = .int


/**
 * Factory method.
 *
 * @param list  The List to iterate over.
 */
  *: factory
    arg s = .string[]
    list_  = s
    index_ = 0
    return
    
    
    /**
     * Returns 1 if more elements are available.
     *
     * @return 1 if there is a next element, 0 otherwise.
     */
  hasNext: method = .int
    return index_ < list_.0
    
    
    /**
     * Returns the next element in the iteration.
     *
     * Advances the cursor.
     *
     * @return The next element, or 0 if none available.
     */
  next: method = .string
    if index_ >= list_.0 then
      return 'error'  -- TODO exception
      
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
    
