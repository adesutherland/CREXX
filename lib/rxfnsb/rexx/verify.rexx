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

namespace rxfnsb expose verify

verify: procedure = .int
       arg instring = .string, intab = .string, match_mode='N', spos=1

   ilen=0
   tlen=0
   char=""
   tab=""
   pos=0

   if match_mode='N' then imatch=0
   else if match_mode='n' then imatch=0
   else if match_mode='M' then imatch=1
   else if match_mode='m' then imatch=1

   Assembler strlen ilen,instring        /* determine string length              */
   Assembler strlen tlen,intab           /* determine table  length              */
   if ilen=0 then return 0
   if tlen=0 then return spos

   spos=spos-1
   if spos<0 then spos=0

   do i=spos to ilen-1                  /* check each byte of input string          */
      assembler strchar char,instring,i /* get next input byte  */
      assembler itos char
      fnd=0
      do j=0 to tlen-1                  /* check each byte of verify table       */
         assembler strchar tab,intab,j  /* get next input byte  */
         assembler itos tab
         if char=tab then do            /* char found in table, check next input char */
            fnd=1                       /* set found  */
            j=tlen                      /* leave loop by setting it to upper limit */
         end
      end
      if fnd=1 & imatch=1 then do       /* if found & match=1 set position and leave outer loop */
         pos=i+1                        /* set found offset to position            */
         i=ilen                         /* leave outer loop */
      end
      if fnd=0 & imatch=0 then do       /* if not found and match=0 set position and leave outer loop */
         pos=i+1                        /* not found offset to position            */
         i=ilen                         /* leave outer loop */
      end
   end

return pos
