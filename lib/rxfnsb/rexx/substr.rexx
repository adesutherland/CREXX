/* Classic REXX Runtime Library */

options levelb /* Written in REXX Level B */

namespace rxfnsb expose substr
import _rxsysb

substr: procedure = .string
  arg string1 = .string, start = .int, len = length(string1) + 1 - start, pad = ' '

/* say 'SUBSTR: start =' start
   say 'SUBSTR: string1 =' string1
   say 'SUBSTR: len =' len
   say 'SUBSTR: pad =' pad
 */

  /* Check arguments early */
  if start < 1 then call raise "syntax", "40.13", start
  if len   < 0 then call raise "syntax", "40.13", len
  if len   = 0 then return ''

  padchar      = 0 /* Is an integer */
  padstring    =""
  outputstring = ""
  inputLength  = 0;
  padLength    = 0;

 /* pad preparation */
  assembler strlen padLength, pad
  if padLength > 1 then call raise "syntax", "40.23", pad /* Invalid pad length */
  if padLength = 0 then pad=' '
  assembler strchar padchar, pad     /* Get pad character code */

  start = start - 1     /* Convert to 0-based offset */

  /* Get string length and calculate how much we can copy */
  assembler strlen inputLength, string1
  inputLength = inputLength - start   /* remaining string length from start position */

  /* Set byte offset for VM (UTF-8 safe) */
  assembler SETSTRPOS string1, start

  /* start copy process  */
  if inputLength > 0 then do
     if len <= inputLength then do      /* Enough characters from string1 */
        assembler substring outputstring, string1, len
     end
     else do                            /* Not enough, copy what we can and pad */
        assembler substring outputstring, string1, inputLength
         do i = 1 to len - inputLength     /* Append the pads */
            assembler appendchar outputstring,padchar
         end
     end
  end
  else do                               /* No characters from string1, only pad */
     do i = 1 to len      /* Append pads */
        assembler appendchar outputstring,padchar
     end
  end
return outputstring