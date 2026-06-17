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

namespace rxfnsb expose right
import _rxsysb

right: procedure = .string
  arg rstring = .string, rlen = .int, pad = ' '

  if rlen = 0 then return ''

  slen=0
  assembler strlen slen,rstring
  if rlen = slen then return rstring

  padchar = ''
  padlength=0

  assembler strlen padLength, pad
  if padLength > 1 then call raise "syntax", "40.23", pad
  assembler strchar padchar, pad            /* Get UTF-8 codepoint */
  diff = rlen - slen                        /* is left padding necessary? */
  if diff > 0 then do                       /* yes */
     padstring = ''
     assembler padstr padstring, padchar, diff /* create left padding string */
     return padstring||rstring
  end
return substr(rstring, slen + 1 - rlen, rlen)
