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

namespace rxfnsb expose x2b

/* X2b(hex-string)  returns bit combination of hex string */

x2b: procedure = .string
 arg hex = .string, slen=-1
 if hex="" then return ""
/*      0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111 */
bittab="0000000100100011010001010110011110001001101010111100110111101111"
trtab="0123456789ABCDEFabcdef "
char=""
rstr=""
hlen=0
offset=0
added=0
assembler strlen hlen,hex
do i=0 to hlen-1
   assembler strchar char,hex,i
   assembler poschar offset,trtab,char  /* position in hex table   */
   if offset<0 then do
      say "hex string contains invalid character "hex
      return ""
   end
   if added=0 & offset=0 then iterate  /* ignore first leading zero */
   added=added+1
   if offset=22 then iterate       /* it's a blank ignore it */
   if offset>15 then offset=offset-6
   offset=offset*4
   /* rstr\="" then rstr=rstr||" " */
   do k=offset to offset+3
      assembler concchar rstr,bittab,k
   end
end
if added=0 then rstr='0000'
return rstr