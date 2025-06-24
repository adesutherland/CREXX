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
  if val='' then return 0
  if type='A' then return verify(val,'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789')=0
  if type='B' then return verify(val,'01 ')=0
  if type='L' then return verify(val,'abcdefghijklmnopqrstuvwxyz')=0
  if type='M' then return verify(val,'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz')=0
  if type='U' then return verify(val,'ABCDEFGHIJKLMNOPQRSTUVWXYZ')=0
  if type='W' then return verify(val, '0123456789')=0
  if type='X' then return verify(val,'ABCDEFabcdef0123456789 ')=0 ## allow also blank
  if type='S' then do
     say "*** Datatype type=S(ymbol) not yet supported"
     return 0
  end
  if type='N' then do
     if verify(val, '0123456789.')>0 then return 0
     else do
        ppi=pos('.', val)
        if ppi=0 then return 1
        if ppi=lastpos('.', val) then return 1
        else return 0
     end
  end
return 0