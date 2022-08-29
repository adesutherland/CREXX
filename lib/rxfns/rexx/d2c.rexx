/* rexx center text  */
options levelb

namespace rxfnsb

d2c: procedure = .string
  arg from = .int, slen=-1
  xlen=0
  if slen=0 then return ''
  if slen>0 then xstr=d2x(from,slen+slen)
     else xstr=d2x(from)

  xstr=x2c(xstr)
  assembler strlen xlen,xstr

  if slen>0 then do
     slen=slen+2  /* double bytes */
      assembler strlen xlen,xstr
     xlen=slen-xlen
  end
return xstr
