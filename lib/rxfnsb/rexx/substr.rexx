/* Classic REXX Runtime Library */
/* substr is UTF8 save! */

options levelb /* Written in REXX Level B */

namespace rxfnsb expose substr
import _rxsysb

substr: procedure = .string
  arg string1 = .string, start = .int, len =-256 , pad = ' '  /* set len default to -256, to avoid a call to the length function */

  inputLength  = 0
  assembler strlen inputLength, string1          /* this is the total string length */
  if len=-256 then len=inputLength + 1 - start   /* len was not set, use the faster assembler instruction to retrieve and calculate the length */

  if start < 1 then call raise "syntax", "40.13", start
  ## if len   < 0 then call raise "syntax", "40.13", len
  if len   <= 0 then return ''

  /* fast path if start position is 1, string can be cut off after length */
  if start=1 then do
     if len = inputLength then return string1  /* same length, return immediately */
     if len<=inputLength then do
        assembler substcut string1,len         /* no length checking necessary, handled in substcut */
        return string1
     end
  end

  /* Initialize registers */
  padchar      = 0  /* Holds Unicode codepoint */
  padstring    = ""
  outputstring = ""
  padLength    = 0

  /* Pad character validation and preparation */
  if pad = '' then pad = ' '
  assembler strlen padLength, pad
  if padLength > 1 then call raise "syntax", "40.23", pad
  assembler strchar padchar, pad   /* padchar= utf8codepoint(pad) */

  /* Convert to 0-based character offset */
  start = start - 1

  /* Calculate remaining input length */
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