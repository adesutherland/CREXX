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

namespace _rxsysb expose _ftrunc

_ftrunc: procedure = .string
       arg innum = .float
   retnum=""
   hlen=0
   i=0
   dp=0
   schar='.'
   char=""
   assembler strchar schar,schar,i   /* get decimal point */
   assembler itos schar
   assembler ftos innum              /* translate input from float to string */
   Assembler strlen hlen,innum       /* determine string length              */

/* step 1 skip behind decimal point decimal point */
   do i=0 to hlen-1                  /* transfer byte by byte, until decimal point */
      assembler strchar char,innum,i /* get next input byte  */
      assembler itos char
      if char=schar then do           /* is it decimal point  */
         dp=i                         /* offset of it         */
         leave                        /* break loop           */
      end
   end
   if dp=0 then return retnum
/* step 2 transfer all fraction digits (up to max fraction) */
   do i=dp+1 to hlen-1               /* loop through remaining string */
      assembler strchar char,innum,i /* fetch next char     */
      assembler itos char
      assembler appendchar retnum,char /* append return string */
   end
return retnum