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

namespace rxfnsb expose lastpos

/* pos(sub-string in string beginning at) */
lastpos: procedure = .int
  arg needle = .string, haystack=.string, upto=0
  if upto<1 then do
     assembler strlen upto,haystack
  end
  nlen=0
  assembler strlen nlen,needle
  if nlen=0      then return 0
  if haystack='' then return 0
  nlen=nlen-1
  foundpos=1
  lastfound=0
  do forever
     assembler strpos foundpos,needle,haystack
     if foundpos = 0 then leave
     if foundpos + nlen> upto then leave    ## nlen-1 calculated above
     lastfound = foundpos
     foundpos=foundpos + 1
  end
return lastfound
