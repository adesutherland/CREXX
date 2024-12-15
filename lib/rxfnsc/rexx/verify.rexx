/* rexx */
  options levelb

namespace rxfnsb expose verify

verify: procedure = .int
       arg instring = .string, intab = .string, match='N', spos=1

   ilen=0
   tlen=0
   char=""
   tab=""
   pos=0

   if match='N' then imatch=0
   else if match='n' then imatch=0
   else if match='M' then imatch=1
   else if match='m' then imatch=1

   Assembler strlen ilen,instring        /* determine string length              */
   Assembler strlen tlen,intab           /* determine table  length              */
   if ilen=0 then return 0
   if tlen=0 then return spos

   spos=spos-1
   if spos<0 then spos=0

   do i=spos to ilen-1                  /* check each byte of input string          */
      assembler strchar char,instring,i /* get next input byte  */
      assembler itos char
      fnd=0
      do j=0 to tlen-1                  /* check each byte of verify table       */
         assembler strchar tab,intab,j  /* get next input byte  */
         assembler itos tab
         if char=tab then do            /* char found in table, check next input char */
            fnd=1                       /* set found  */
            j=tlen                      /* leave loop by setting it to upper limit */
         end
      end
      if fnd=1 & imatch=1 then do       /* if found & match=1 set position and leave outer loop */
         pos=i+1                        /* set found offset to position            */
         i=ilen                         /* leave outer loop */
      end
      if fnd=0 & imatch=0 then do       /* if not found and match=0 set position and leave outer loop */
         pos=i+1                        /* not found offset to position            */
         i=ilen                         /* leave outer loop */
      end
   end

return pos
