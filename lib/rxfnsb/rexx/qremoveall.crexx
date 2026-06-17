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

namespace rxfnsb expose qremoveall

/* ------------------------------------------------------------------
 *  Function:  QREMOVEALL
 *  Purpose:   Removes all delimited segments from a string,
 *             respecting quotes and nested pairs.
 *
 *  Usage:     clean = QREMOVEALL(open, close, text [, mode])
 *
 *  Arguments:
 *     open   - Opening delimiter (string, one or more characters)
 *     close  - Matching closing delimiter
 *     text   - Source string to clean
 *     mode   - (optional) 'exclusive' or 'inclusive'
 *              'inclusive' (default) removes including delimiters
 *              'exclusive' removes only the inner content
 *
 *  Returns:
 *     A copy of the text with all delimited blocks removed.
 *
 *  Notes:
 *     - Quote-safe and nesting-aware (via QEXTRACTALL)
 *     - Default behavior removes full matched segments
 *     - For exclusive mode, keeps delimiters but removes inner content
 *     - Multiple nested or sequential blocks supported
 *  ------------------------------------------------------------------
 */
qremoveall: procedure=.string
  arg open=.string, close=.string, text=.string, mode='I'

  out = text
  blocks = qextractall(open, close, text, mode)

  do i = 1 to blocks.0
     out = changestr(blocks.i, out, '')
  end
return out