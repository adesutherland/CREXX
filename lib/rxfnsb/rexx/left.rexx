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

namespace rxfnsb expose left
import _rxsysb

left: procedure=.string
  arg lstring=.string, leftlen=0, pad=' '
  llen=0
  assembler strlen llen,lstring

  if leftlen<=0 | llen=0 then return ''     /* one of the length is zero, return empty string */
  if leftlen = llen then return lstring     /* no length change, return original */
  if leftlen < llen then do                 /* requested length is < then string length, just cut it off */
     assembler substcut lstring,leftlen     /* just cut off string */
     return lstring                         /* and return it */
  end
 /* left length > string length, this needs padding */
  padchar=''
  padstring=''
  padlength=0
  assembler strlen padLength, pad
  if padLength > 1 then call raise "syntax", "40.23", pad
  assembler strchar padchar, pad           /* padchar= utf8codepoint(pad) */
  leftlen=leftlen-llen                     /* how many padding chars needed */
  assembler padstr padstring, padchar, leftlen  /* create pad string */
return lstring||padstring                  /* return padded string */
