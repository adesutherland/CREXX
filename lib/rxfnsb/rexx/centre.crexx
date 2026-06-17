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

/* rexx center text  */
/* CENTER and CENTRE are exact copies, any change must be reflected in both functions */
/* todo fix this bit of ugliness when crexx allows */
options levelb

namespace rxfnsb expose centre

centre: procedure = .string
  arg expose string = .string, centlen = .int,  pad = " "

padstr=""
offset=0
slen=0
cpad=""

/* make sure just to take first char */
assembler strchar cpad,pad,offset
assembler load pad,""
assembler appendchar pad,cpad
/* calculate number of pad chars to added as prefix and suffix */
assembler strlen slen,string
padlen=centlen-slen
assembler idiv padlen,padlen,2
if padlen=0 then return string   /* if nothing to add return original string */

if padlen<0 then newstr=substr(string,-padlen+1,centlen," ")
else do
/* create padding string */
   padstr=copies(pad,padlen)
   newstr=padstr||string||padstr
   assembler strlen slen,newstr
   if slen<centlen then newstr=newstr||pad  /* in case of uneven center length */
end

return newstr
