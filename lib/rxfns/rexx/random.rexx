/* rexx random */
options levelb

namespace rxfnsb expose random

random: procedure = .int
arg expose min=0, max=-1,seed = -1    /* seed = time()%(3600*24) */
if max<0 then max=999
if min<0 then do
   call raise "syntax", "40.??","invalid minimum value ("min","max")"
   return -999999
end
if min>max then do
   call raise "syntax", "40.??","minimum value greater than maximum value ("min","max")"
  return -9999999
end
rx1=0
assembler irand rx1,seed  /* seed -1, will be handled by irand instruction */
rx1=rx1//(max-min+1) + min
return rx1

#raise: procedure = .int
#  arg type = .string, code = .string, parm1 = .string

  