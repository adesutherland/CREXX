/* rexx */
options levelb
/* d2x(hex-string)  returns hex string of integer  */

namespace rxfnsb expose d2x

d2x: procedure = .string
  arg xint = .int, slen=-1
  if slen=0 then return ""
  if slen>20 then slen=20
  trtab="0123456789ABCDEF"
  hlen=0
  sign=0
  yint=0
  rint=0
  rstr=""
  xstr=""
  sint=""

  if xint<0 then do
     sign=1
     dc=8
     do j=1 to 3
        dc=dc*16
     end
     xint=dc+xint
  end
  do i=0 to 64
     assembler idiv yint,xint,16
     assembler imod rint,xint,16
     xint=yint

     if xint>0 then do
        assembler strchar sint,trtab,rint
        assembler appendchar rstr,sint
     end
     else do       /* xint=0, this is the highest number then!   */
        if sign>0 then assembler ior rint,rint,8
        assembler strchar sint,trtab,rint
        assembler appendchar rstr,sint
        assembler strlen hlen,rstr

        do i=1 to hlen   /* now reverse the string */
           j=hlen-i
           assembler concchar xstr,rstr,j
        end
   /*     if hlen//2=1 then xstr='0'xstr */ /* dropped 2 bytes adjustment */
        if slen>0 then do
           if sign>0 then xstr=right(xstr,slen,'F')
             else xstr=right(xstr,slen,'0')
        end
        return xstr
     end
  end
return ""
