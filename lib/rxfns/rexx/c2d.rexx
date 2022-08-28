/* rexx center text  */
options levelb
namespace rxfnsb

c2d: procedure = .int
  arg from = .string
  stx=""
  fz=""
  len=0
  assembler strlen len,from
  if len=0 then return 0
  do  i=0 to len-1
      assembler hexchar fz,from,i
      stx=stx||fz
  end
  return x2d(stx)


