/* rexx */
  options levelb

trunc: procedure = .string
       arg innum = .string, fraction = 0
   retnum=""
   hlen=0
   i=0
   ifrac=0
   dp=0
   schar='.'
   char=""
   assembler strchar schar,schar,i   /* get decimal point */
   assembler itos schar

 /*  assembler ftos innum      */        /* translate input from float to string */
   Assembler strlen hlen,innum       /* determine string length              */

/* step 1 transfer all digits before decimal point */
   do i=0 to hlen-1                  /* transfer byte by byte, until decimal point */
      assembler strchar char,innum,i /* get next input byte  */
      assembler itos char
      if char=schar then do          /* is it decimal point  */
         dp=i                        /* if yes, save offset  */
         leave                       /* set i hlen, to abort do loop */
      end
      else assembler appendchar retnum,char /* if no, transfer byte  */
   end
   if i=0 then retnum="0"
   if fraction=0 then return retnum  /* no fraction digits requested */
/* step 2 transfer all fraction digits (up to max fraction) */
   assembler appendchar retnum,schar /* set decimal point   */
   do i=dp+1 to hlen-1               /* loop through remaining string */
      assembler strchar char,innum,i /* fetch next char     */
      assembler itos char
      assembler appendchar retnum,char /* append return string */
      ifrac=ifrac+1                    /* increase fraction count */
      if ifrac>=fraction then leave     /* if limit reached leave loop */
   end
/* step 3 add padding bytes (if necessary) */
    if ifrac< fraction then do
      retnum=left(retnum,fraction+dp+1,'0')
   end

return retnum

left: procedure = .string
  arg string1 = .string, len = .int, pad = '0'