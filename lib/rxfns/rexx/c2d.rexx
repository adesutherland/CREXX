/* rexx center text  */

options levelb
c2d: procedure = .string
  arg from = .string
  stx=""
  fz=""
  len=0
  assembler strlen len,from
  if len=0 then return ""
  do  i=0 to len-1
      assembler hexchar fz,from,i
      stx=stx||fz
  end
  return reradix(stx,16,10)

  /* function prototype */
  
  reradix: procedure = .string
  arg Subject = .string, FromRadix = .int, ToRadix = .int
