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

namespace rxfnsb expose qextractpair

qextractpair: procedure=.string
   arg open=.string, close=.string, text=.string, start=1, mode='X'

   mode = upper(left(mode,1))
   if mode = 'E' then mode = 'X'
   if mode \= 'I' & mode \= 'X' & mode \= 'C' then mode = 'X'

   lenOpen  = length(open)
   lenClose = length(close)

   /* ------------------------------------------------------------
    * Comment mode:
    *   literal search only, no quote awareness, no nesting
    *   intended for comment blocks
    * ------------------------------------------------------------
    */
   if substr(open,1,2)='/*' then do
      firstPos = pos(open, text, start)
      if firstPos = 0 then return ''

      nextClose = pos(close, text, firstPos + lenOpen)
      if nextClose = 0 then return ''

      if mode = 'X' then return substr(text, firstPos + lenOpen, nextClose - firstPos - lenOpen)
      else return substr(text, firstPos, nextClose - firstPos + lenClose)
   end

   /* ------------------------------------------------------------
    * Normal mode:
    *   nested balanced delimiters
    *   uses QPOS() so delimiters in quoted strings are ignored
    * ------------------------------------------------------------ */
   firstPos = qpos(open, text, start)
   if firstPos = 0 then return ''

   depth = 1
   posn  = firstPos + lenOpen

   do forever
      nextOpen  = qpos(open,  text, posn)
      nextClose = qpos(close, text, posn)

      if nextOpen = 0 & nextClose = 0 then return ''

      if nextClose = 0 | (nextOpen > 0 & nextOpen < nextClose) then do
         depth = depth + 1
         posn  = nextOpen + lenOpen
      end
      else do
         depth = depth - 1
         if depth = 0 then do
            if mode = 'X' then return substr(text, firstPos + lenOpen, nextClose - firstPos - lenOpen)
            else return substr(text, firstPos, nextClose - firstPos + lenClose)
         end
         posn = nextClose + lenClose
      end
   end
return ''