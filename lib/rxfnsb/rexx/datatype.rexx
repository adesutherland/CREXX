/*
 * rexx built-in function Datatype
 */
options levelb

namespace rxfnsb expose datatype

datatype: procedure=.string
  arg val=.string, type=''

  val = strip(val)
  if val = '' then return 'CHAR'           ## 'EMPTY'

  /* Check for digits only */
  if verify(val, '0123456789') = 0 then return 'NUM'      ## plain INTEGER

  /* Check for number with one optional dot */
  if verify(val, '0123456789.') = 0 then do
    if pos('.', val) = lastpos('.', val) & pos('.', val) > 0 then return 'NUM'
  end
return 'CHAR'
/* ------------------------------------------
 * maybe we use this later
  /* Check for alpha only */
  if verify(val, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz') = 0 then return 'CHAR    ## we could distinguish 'ALPHA' from 'ALPHANUM'

  /* Check for alphanum */
  if verify(val, 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789') = 0 then return 'ALPHANUM'
return 'MIXED'
 * ------------------------------------------
 */