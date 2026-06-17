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
namespace rxfnsb expose x2d

/* X2d(hex-string)  returns decimal number of hex string */

x2d: procedure = .int
  arg hex = .string, slen=-1
  trtab="0123456789ABCDEFabcdef"
  hlen=0
  char=0
  dec=0
  sign=0
  offset=0
  len = 0
  if slen=0 then return 0      /* no hex input return 0    */
  if slen>20 then slen=20      /* may be up to 20 chars, inluding sign */
  assembler strlen hlen,hex    /* get length of hex string */
  if slen > -1 then do
     hex=right(hex,slen,"0")  /* if length parm set, treat hex as signed integer */
     len = slen
  end
  else len=hlen                          /* else it is an unsigned integer */
  do i=0 to len-1                        /* loop through hex string */
     assembler strchar char,hex,i         /* fetch one byte          */
     assembler poschar offset,trtab,char  /* position in hex table   */
     if offset<0 then do
        say "hex string contains invalid character "hex
        return 0
     end
     if offset>15 then offset=offset-6    /* translate lower case to upper case hex */
     char=offset
  /* if it is first byte then check if sign is set, and convert number */
     if i=0 then do
        if char>=8 & slen > -1 then do /* does it contain sign? */
           sign=1                   /* keep sign             */
           char=char-8              /* switch off sign       */
        end
     end
     dec=dec*16+char        /* cumulate fetched char in result integer */
  end
/* hex string has been processed, check if there was a sign */
  if sign=1 then do        /* was sign set?    */
     char=8                /* build complement */
     do i=1 to len-1
        char=char*16
     end
     dec=dec-char          /* create integer out of complement */
  end
return dec
