/*
 * rexx built-in function Datatype
 */
options levelb

namespace rxfnsb expose datatype
/* --------------------------------------------------------------------------------------------
 * DATATYPE(string[, type]) — Returns information about the type of the provided string.
 *
 * If the optional 'type' parameter is omitted:
 *     - Returns 'NUM' if the string is numeric
 *     - Returns 'CHAR' if the string is non-numeric
 *
 * If the 'type' parameter is provided:
 *     - Returns 1 if the string matches the specified type
 *     - Returns 0 otherwise
 * supported types are
 *   A (Alphanumeric)
 *   B (Binary)
 *   L (lowercase)
 * ...
 * --------------------------------------------------------------------------------------------
 */
datatype: procedure=.string
  arg val=.string, type=""

  val = strip(val)
  if type='' then do
      if val = '' then return 'CHAR'           ## 'EMPTY'
    /* Check for digits only */
      if verify(val, '0123456789') = 0 then return 'NUM'      ## plain INTEGER
    /* Check for number with one optional dot */
      if verify(val, '0123456789.') = 0 then do
         if pos('.', val) = lastpos('.', val) & pos('.', val) > 0 then return 'NUM'
      end
      return 'CHAR'
  end
## Here we have a type parameter defined
  type=upper(substr(type,1,1))

  if type='A' then return (length(val)>0 & verify(val,'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789')=0)
  if type='B' then return (length(val)=0 | verify(val,'01 ')=0)
  if type='L' then return (length(val)>0 & verify(val,'abcdefghijklmnopqrstuvwxyz')=0)
  if type='M' then return (length(val)>0 & verify(val,'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz')=0)
  if type='U' then return (length(val)>0 & verify(val,'ABCDEFGHIJKLMNOPQRSTUVWXYZ')=0)
  if type='W' then do
     if length(val)=0 then return 0
     if verify(val, '0123456789')=0 then return 1   ## handle simple whole numbers
     exp=isExponential(val)
     if exp<1 then do         /* is not an exponential number, further check */
        if verify(val, "0123456789.")>0 then return 0
        return iswholeNumber(val)
     end
     return exp                /* it was exponential, return result of check  */
  end
  if type='X' then return verify(val,'ABCDEFabcdef0123456789 ')=0 ## allow also blank
  if type='S' then return (length(val)>0 & verify(val,'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!?_@##$')=0)
  if type='N' then do
     if length(val)=0 then return 0
     if verify(val, '0123456789.')>0 then return 0
     else do
        ppi=pos('.', val)
        if ppi=0 then return 1
        if ppi=lastpos('.', val) then return 1
        else return 0
     end
  end
return 0
isExponential: procedure=.int
  arg val=.string
  val=upper(val)
  posE = pos("E", val)
  if posE = 0 then return -1   ## no exponent found, further check necessary
  if posE = 1 then return 0    ## invalid exponent position, can't be a number

  number   = substr(val,1,posE-1)
  if verify(number, "0123456789.") > 0 then return 0     ## number part is not a number
  if pos('.',number)\=lastpos('.',number) then return 0  ## invalid number part

  exponent = substr(val, posE + 1)
  if length(exponent) < 1 | length(exponent) > 4 then return 0
  sign = substr(exponent, 1, 1)
  if sign = "+" | sign = "-" then exponentDigits = substr(exponent, 2)
  else do
     sign='+'   ## no sign means +
     exponentDigits = exponent
  end
  if verify(exponentDigits, "0123456789") = 0 then do
     if length(exponentDigits) >= 1 & length(exponentDigits) <= 3 then nop  ## valid exponent
     else return 0  ## invalid exponent can't be a whole number
  end
/* coming here means we have a valid exponential number */
  if verify(number,'0.')=0 then return 1  ## if number is 0, it it is always a whole number, whatever the exponent is
  if pos('.',number)=0 & sign='+' then return 1
  tstnum=number+0.0
  if sign='+' then tstnum=tstnum*(10**exponentDigits)
  else tstnum=tstnum/(10**exponentDigits)
return iswholeNumber(tstnum)

isWholeNumber: procedure=.int
  arg num=.string
  threshold = 0.0000005    ## 123.0000003
  ppi=pos('.',num)
  if ppi=0 then return 1   ## no decimal point , so it must be a plain number
  rhs='0.'substr(num,ppi+1)
  remainder=0.0+rhs
  remainder = abs(remainder)
  if remainder < threshold then return 1
return 0    ## else
