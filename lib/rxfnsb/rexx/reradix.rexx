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

/*
 *.  ReRadix
 *   Converts Arg(1) from radix Arg(2) to radix Arg(3) 
 *   Radix range is 2-16.  Conversion is via decimal
 *   After Brian Marks' version 
 */ 
options levelb numeric_classic

namespace _rxsysb expose reradix
import rxfnsb

reradix: procedure = .string
arg subject = .string, FromRadix = .int, ToRadix = .int

subject = upper(subject)
 integer=0
 do j=1 to length(subject)
   /* Individual digits have already been checked for range. */
   integer=integer*FromRadix+pos(substr(subject,j,1),'0123456789ABCDEF')-1
   /* This test not for standard. */
    if pos('E',integer)>0 then do
      say "ReRadix unable"
      return "?"
    end
 end
 r=''
 if integer=0 then r='0'
 do while integer>0
    r=substr('0123456789ABCDEF',1+integer//ToRadix,1)||r
   integer=integer%ToRadix
 end
 /* When between 2 and 16, there is no zero suppression. */
 if FromRadix=2 & ToRadix=16 then
   r=right(r,(length(subject)+3)%4,'0')
 else if FromRadix=16 & ToRadix=2 then
   r=right(r,length(subject)*4,'0')
 return r
