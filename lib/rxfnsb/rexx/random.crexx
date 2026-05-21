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

/* rexx random */
options levelb numeric_classic
import _rxsysb

namespace rxfnsb expose random

random: procedure = .int
arg expose min=0, max=-1,seed = -1    /* seed = time()%(3600*24) */
if max<0 then max=999
if min<0 then do
   call raise "syntax", "40.??","invalid minimum value ("min","max")"
   return -999999
end
if min>max then do
  call raise "syntax", "40.??","minimum value greater than maximum value ("min","max")"
  return -9999999
end
rx1=0
assembler irand rx1,seed  /* seed -1, will be handled by irand instruction */
rx1=rx1//(max-min+1) + min
return rx1

#raise: procedure = .int
#  arg type = .string, code = .string, parm1 = .string

  