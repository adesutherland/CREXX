  /* rexx */
  options levelb

  namespace rxfnsb

strip: procedure = .string
       arg instr = .string, option = "B", schar= " "
   retstr=""
   hlen=0
   i=0
   char=""

   assembler strchar schar,schar,i
   assembler itos schar
 /* strip leading chars */
   option=substr(option,1,1)
   if option='l' then option='L'
   if option='t' then option='T'
   if option='b' then option='B'
   if option=' ' then option='B'

   retstr=instr
   if option='L' | option='B' then do    /* check leading blanks */
      Assembler strlen hlen,retstr
      do i=0 to hlen-1
         assembler strchar char,retstr,i
         assembler itos char
         if char\=schar then do
            retstr=substr(retstr,i+1)     /* place none byte char and leave loop  */
            leave
         end
      end
   end
   if option='T' | option='B' then do   /* check trailing blanks */
      Assembler strlen hlen,retstr
      do i=hlen-1 to 0 by -1
         Assembler strchar char,retstr,i
         Assembler itos char
         if char\=schar then return substr(retstr,1,i+1)
      end
   end
return retstr
