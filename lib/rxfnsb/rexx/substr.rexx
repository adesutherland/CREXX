/* Classic REXX Runtime Library */

options levelb /* Written in REXX Level B */

namespace rxfnsb expose substr
import _rxsysb

/* Substr() Procedure */
substr: procedure = .string
  arg string1 = .string, start = .int, len = length(string1) + 1 - start, pad = ' '

  padchar = 0 /* Is an integer */
  outputstring = ""
  inputLength = 0;
  padLength = 0;
/*
 say 'SUBSTR: start = 'start
 say 'SUBSTR: string1 ="'string1'"'
 say 'SUBSTR: len = 'len
 say 'SUBSTR: pad ="'pad'"'
*/

  /* Check Start */
  if start < 1 then call raise "syntax", "40.13", start /* Invalid start */

  /* Check Length */
  if len < 0 then call raise "syntax", "40.13", len /* Invalid start */
  if len=0 then return ''
  /* Check length of pad */
  assembler strlen padLength,pad;
  if padLength > 1 then call raise "syntax", "40.23", pad /* Invalid pad length */
  else if padLength = 0 then call raise "syntax", "40.23", "SUBSTR argument 4 must be a single character; found ''"

  /* Get the Length of the input string */
  assembler strlen inputLength,string1
  inputLength = inputLength - start+1;

  if inputLength > 0 then do             /* Yes there are characters needed from string1 */
     if len <= inputLength then do        /* Just copy from string1 - no padding needed */
        start = len * 4294967296+start    /* move length to the upper part of the integer, offset to the lower part */
        assembler substring outputstring,string1,start
     end
     else do                              /* Copy all of string1 and then pad */
        start = inputLength * 4294967296+start         ## move length to the upper part of the integer, offset to the lower part
        assembler substring outputstring,string1,start
        /* Then add pads, get Pad Char as integer */
        if padLength = 0 then pad = " "
        assembler strchar padchar,pad,padchar /* padchar is set to 0 so can use it as char pos */
        do i = 1 to len - inputLength     /* Append the pads */
           assembler appendchar outputstring,padchar
        end
     end
  end
  else do   /* Nothing from string 1 - just Pad */
     /* Get Pad Char as integer */
     if padLength = 0 then pad = " "
     assembler strchar padchar,pad,padchar /* padchar is set to 0 so can use it as char pos */
     do i = 1 to len      /* Append pads */
        assembler appendchar outputstring,padchar
     end
  end
/* Done */
  return outputstring
