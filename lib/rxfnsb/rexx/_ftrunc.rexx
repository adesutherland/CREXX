/* rexx */
options levelb

namespace _rxsysb expose _ftrunc

_ftrunc: procedure = .string
       arg innum = .float
   retnum=""
   hlen=0
   i=0
   dp=0
   schar='.'
   char=""
   assembler strchar schar,schar,i   /* get decimal point */
   assembler itos schar
   assembler ftos innum              /* translate input from float to string */
   Assembler strlen hlen,innum       /* determine string length              */

/* step 1 skip behind decimal point decimal point */
   do i=0 to hlen-1                  /* transfer byte by byte, until decimal point */
      assembler strchar char,innum,i /* get next input byte  */
      assembler itos char
      if char=schar then do           /* is it decimal point  */
         dp=i                         /* offset of it         */
         leave                        /* break loop           */
      end
   end
   if dp=0 then return retnum
/* step 2 transfer all fraction digits (up to max fraction) */
   do i=dp+1 to hlen-1               /* loop through remaining string */
      assembler strchar char,innum,i /* fetch next char     */
      assembler itos char
      assembler appendchar retnum,char /* append return string */
   end
return retnum