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
namespace rxfnsb expose arrayfind

/* --------------------------------------------------------------------------------------
 * arrayfind(find, array[, from[, case]])
 *
 * Searches an array of strings for the first element containing a substring.
 *
 * Arguments:
 *   find   - substring to search for
 *   array  - array of strings (array[0] = element count)
 *   from   - starting array position (default: 1)
 *   case   - case sensitivity flag:
 *            1 = case-sensitive (default)
 *            0 = case-insensitive
 *
 * Returns:
 *   Index (1-based) of the first matching array element,
 *   or 0 if no match is found.
 * --------------------------------------------------------------------------------------
 */
arrayfind: procedure=.int
  arg find=.string,array=.string[],from=1,case=1
  if from < 1 then from = 1
  if from > array[0] then return 0

  if case=1 then do i=from to array[0]     /* case sensitive search */
     if pos(find,array[i])>0 then return i
  end
  else do       /* case insensitive search */
      ufind=upper(find)
      do i=from to array[0]
         if pos(ufind,upper(array[i]))>0 then return i
       end
  end
return 0