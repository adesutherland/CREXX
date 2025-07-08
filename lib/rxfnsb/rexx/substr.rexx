/* Classic REXX Runtime Library */

options levelb /* Written in REXX Level B */

namespace rxfnsb expose substr
import _rxsysb

substr: procedure = .string
  arg string1 = .string, start = .int, len = length(string1) + 1 - start, pad = ' '

  if start < 1 then call raise "syntax", "40.13", start
  if len   < 0 then call raise "syntax", "40.13", len
  if len   = 0 then return ''

  /* Initialize registers */
  padchar      = 0  /* Holds Unicode codepoint */
  padstring    = ""
  outputstring = ""
  inputLength  = 0
  padLength    = 0

  /* Pad character validation and preparation */
  if pad = '' then pad = ' '
  assembler strlen padLength, pad
  if padLength > 1 then call raise "syntax", "40.23", pad
  assembler strchar padchar, pad   /* padchar= utf8codepoint(pad) */

  /* Convert to 0-based character offset */
  start = start - 1

  /* Calculate remaining input length */
  assembler strlen inputLength, string1          /* this is the total string length */
  inputLength = inputLength - start              /* calculate usable length after positioning substr */

  /* Set byte offset for VM based on character index (UTF-8 safe) */
  assembler SETSTRPOS string1, start
  /* Copy or pad */
  if inputLength <= 0 then do  /* just padding required */
     assembler padstr outputstring, padchar, len
  end
  else do                      /* inputLength > 0   */
     if len <= inputLength then do
        assembler substring outputstring, string1, len
     end
     else do
        assembler substring outputstring, string1, inputLength
        pcount = len - inputLength
        assembler padstr outputstring, padchar, pcount
     end
  end
  return outputstring
