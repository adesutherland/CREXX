/* Classic REXX Runtime Library */

options levelb /* Written in REXX Level B */

/* decls */
/* Raise() Internal Function to Raise a runtime error */
raise: procedure = .int
  arg type = .string, code = .string, parm1 = .string

/* Length() Procedure */
length: procedure = .int
  arg expose string1 = .string /* Pass by reference */

/* Substr() Procedure */
substr: procedure = .string
arg string1 = .string, start = .int, length = 0, pad = ' '
/* Substr() Procedure */

  padchar = 0 /* Is an integer */
  output = ''
  inputLength = 0;
  padLength = 0;

  /* Check Start */
  if start < 1 then call raise "syntax", "40.??", start /* Invalid start */
  start = start - 1 /* 1 base to zero base */

  /* Check length of pad */
  assembler strlen padLength,pad;
  if padLength > 1 then call raise "syntax", "40.23", pad /* Invalid pad length */

  /* Get the Length of the input string */
  assembler strlen inputLength,string1
  inputLength = inputLength - start;

  if inputLength > 0 then do
    /* Yes there are characters needed from string1 */
    if length <= inputLength then do
      /* Just copy from string1 - no padding needed */
      do i = start to start + length - 1
         assembler concchar output,string1,i
      end
    end
    else do
      /* Copy all of string1 and then pad */
      do i = start to start + inputLength - 1
         assembler concchar output,string1,i
      end

      /* Then add pads */
      /* Get Pad Char as integer */
      if padLength = 0 then pad = " "
      assembler strchar padchar,pad,padchar /* padchar is set to 0 so can use it as char pos */
      /* Append the pads */
      do i = 1 to length - inputLength
        assembler appendchar output,padchar
      end
    end
  end

  else do
    /* Nothing from string 1 - just Pad */
    /* Get Pad Char as integer */
    if padLength = 0 then pad = " "
    assembler strchar padchar,pad,padchar /* padchar is set to 0 so can use it as char pos */
    /* Append pads */
    do i = 1 to length
      assembler appendchar output,padchar
    end
  end

  /* Done */
  return output
