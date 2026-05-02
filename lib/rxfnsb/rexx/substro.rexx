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

/* Classic REXX Runtime Library */

options levelb /* Written in REXX Level B */

namespace rxfnsb expose substro
import _rxsysb

/* Substr() Procedure */
substro: procedure = .string
  arg string1 = .string, start = .int, len = length(string1) + 1 - start, pad = ' '

  padchar = 0 /* Is an integer */
  outputstring = ''
  inputLength = 0;
  padLength = 0;

/* say 'SUBSTR: start =' start */
/* say 'SUBSTR: string1 =' string1 */
/* say 'SUBSTR: len =' len */
/* say 'SUBSTR: pad =' pad */

  /* Check Start */
  if start < 1 then call raise "syntax", "40.13", start /* Invalid start */
  start = start - 1 /* 1 base to zero base */

  /* Check Length */
  if len < 1 then call raise "syntax", "40.13", len /* Invalid start */

  /* Check length of pad */
  assembler strlen padLength,pad;
  if padLength > 1 then call raise "syntax", "40.23", pad /* Invalid pad length */
  else if padLength = 0 then call raise "syntax", "40.23", "SUBSTR argument 4 must be a single character; found ''"


  /* Get the Length of the input string */
  assembler strlen inputLength,string1
  inputLength = inputLength - start;

  if inputLength > 0 then do
    /* Yes there are characters needed from string1 */
    if len <= inputLength then do
      /* Just copy from string1 - no padding needed */
      do i = start to start + len - 1
        assembler concchar outputstring,string1,i
      end
    end
    else do
      /* Copy all of string1 and then pad */
      do i = start to start + inputLength - 1
        assembler concchar outputstring,string1,i
      end

      /* Then add pads */
      /* Get Pad Char as integer */
      if padLength = 0 then pad = " "
      assembler strchar padchar,pad,padchar /* padchar is set to 0 so can use it as char pos */
      /* Append the pads */
      do i = 1 to len - inputLength
        assembler appendchar outputstring,padchar
      end
    end
  end

  else do
    /* Nothing from string 1 - just Pad */
    /* Get Pad Char as integer */
    if padLength = 0 then pad = " "
    assembler strchar padchar,pad,padchar /* padchar is set to 0 so can use it as char pos */
    /* Append pads */
    do i = 1 to len
      assembler appendchar outputstring,padchar
    end
  end

  /* Done */
  return outputstring
