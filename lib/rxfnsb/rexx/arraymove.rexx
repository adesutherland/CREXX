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

namespace rxfnsb expose arraymove

/* ----------------------------------------------------------------------
 * ARRAYMOVE(array, from, count, to)
 *
 * Moves COUNT elements starting at FROM so that they begin at TO.
 * Elements are shifted to close/open gaps as required.
 *
 * Convention:
 *   - array[0] holds number of elements (maintained automatically)
 *   - elements are in array[1] .. array[array[0]]
 *
 * Returns:
 *   The current number of elements in ARRAY (array[0])
 * ----------------------------------------------------------------------
 */
arraymove: procedure=.int
  arg expose array=.string[], from=1, count=0, tto=1

  hi = array[0]
  if hi < 1 then return hi
  if count < 1 then return hi
  if from < 1 then return hi
  if from > hi then return hi

/* clamp count */
  if from + count - 1 > hi then count = hi - from + 1
  if count < 1 then return hi

/* clamp destination: allow append at end */
  if tto < 1 then tto = 1
  if tto > hi + 1 then tto = hi + 1

/* if destination lies inside the block, nothing to do */
  if tto >= from & tto <= from + count then return hi

/* copy the block to a temporary array */
  tmp = .string[]
  do i = 1 to count
     tmp[i] = array[from + i - 1]
  end

/* delete original block (close gap) */
  do i = from + count to hi
     array[i - count] = array[i]
  end
  do i = hi - count + 1 to hi
     array[i] = ''
  end

/* adjust destination if it was after the removed block */
  if tto > from then tto = tto - count

/* insert gap at destination */
  hi2 = array[0]                 /* after delete, size should be hi-count */
  do i = hi2 to tto by -1
     array[i + count] = array[i]
  end
  do i = 0 to count - 1
     array[tto + i] = tmp[i + 1]
  end

return array[0]
