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

/* rexx */
options levelb

namespace rxfnsb expose arrayinsert
/* ----------------------------------------------------------------------
 * ARRAYINSERT(array, from, count[, default])
 *
 * Opens a gap of COUNT elements in ARRAY at position FROM.
 *
 * Elements at ARRAY[FROM] and above are shifted upward to make room.
 * The newly created slots ARRAY[FROM] .. ARRAY[FROM+COUNT-1]
 * are initialised to DEFAULT (default "").
 *
 * Convention:
 *   - array[0] holds the number of elements
 *   - elements are in array[1] .. array[array[0]]
 *
 * Returns:
 *   The new number of elements in ARRAY
 * ----------------------------------------------------------------------
 */
arrayinsert: procedure=.int
  arg expose array=.string[], from=1, count=0, default=""

  hi = array[0]
  if count < 1 then return hi
  if from < 1 then return hi

  /* clamp: inserting beyond end means append at end+1 */
  if from > hi + 1 then from = hi + 1

  /* shift up to make room (work backwards to avoid overwriting) */
  do i = hi to from by -1
    array[i + count] = array[i]
  end

  /* create the empty slots */
  do i = from to from + count - 1
    array[i] = default
  end

return array[0]