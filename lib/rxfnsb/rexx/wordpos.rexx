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

/* rexx wordpos searches for string and returns word position */
options levelb

namespace rxfnsb expose wordpos

wordpos: procedure = .int
  arg expose search = .string, string = .string, start = 1

tlen=0
assembler strlen tlen,string
if tlen=0 then return 0
assembler strlen tlen,search
if tlen=0 then return 0

wnum=words(string)
if wnum=0 then return 0
snum=words(search)
if snum=0 then return 0

search=strip(search)

startpos=wordindex(string,start)  /* calculate wordpos */
do i=start to wnum
   if snum=1 then do
      if abbrev(word(string,i),search)>0 then return i
   end
   else do
      if pos(search,string,startpos) = startpos then return i
      startpos=wordindex(string,i+1)
      if startpos=0 then leave
   end
end
return 0
