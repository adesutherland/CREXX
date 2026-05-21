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
options levelb numeric_classic

namespace rxfnsb expose x2c

/* X2c(hex-string)  returns character representation of hex string */

x2c: procedure = .string
  arg hex = .string
  trtab="0123456789ABCDEFabcdef"
  slen=0
  char1=0
  char2=0
  off1=0
  off2=0
  binary=""
  byte=0
  hexi=""
  blank=" "
  assembler dropchar hexi,hex,blank
  assembler strlen slen,hexi    /* get length of hex string */

  if slen//2=1 then do
     hexi='0'hexi
     slen=slen+1
  end

  do i=0 to slen-1 by 2                   /* loop through hex string */
     assembler strchar char1,hexi,i        /* fetch one byte          */
     assembler poschar off1,trtab,char1   /* position in hex table   */
     j=i+1
     assembler strchar char2,hexi,j        /* fetch one byte          */
     assembler poschar off2,trtab,char2   /* position in hex table   */

     if off1<0 | off2<0 then do           /* no hex char? error!     */
        say "hex string contains invalid character "hexi
        return 0
     end
     if off1>15 then off1=off1-6    /* translate lower case to upper case hex */
     if off2>15 then off2=off2-6    /* translate lower case to upper case hex */
     assembler ishl byte,off1,4     /* move 1. char to left hand part of byte */
     byte=byte+off2                 /* move 2. char to right hand part of byte*/
     assembler appendchar binary,byte /* append to result                     */
  end
 return binary  /* return result */
