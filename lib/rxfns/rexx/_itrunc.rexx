/* rexx */
options levelb

namespace _rxsysb

_itrunc: procedure = .string
       arg innum = .float
   retnum=""
   hlen=0
   schar='.'
   char=""
   i=0
   assembler strchar schar,schar,i   /* get decimal point */
   assembler itos schar
   assembler ftos innum              /* translate input from float to string */
   Assembler strlen hlen,innum       /* determine string length              */
/* transfer all digits before decimal point */
   do i=0 to hlen-1                  /* transfer byte by byte, until decimal point */
      assembler strchar char,innum,i /* get next input byte  */
      assembler itos char
      if char=schar then leave       /* is it decimal point  */
      else assembler appendchar retnum,char /* if no, transfer byte  */
   end
return retnum