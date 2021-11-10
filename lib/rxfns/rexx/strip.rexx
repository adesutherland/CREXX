  /* rexx */
  options levelb

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
   if option='L' | option='B' then do   /* check leading blanks */
      Assembler strlen hlen,retstr
      do i=0 to hlen-1   /* by -1 not yet supported */
         assembler strchar char,retstr,i
         assembler itos char
         if char\=schar then do
            retstr=substr(retstr,i+1)     /* there are leading blanks  */
            i=hlen
         end
      end
   end
   if option='T' | option='B' then do   /* check trailing blanks */
      Assembler strlen hlen,retstr
      j=hlen
      do i=0 to hlen-1   /* by -1 not yet supported */
         j=j-1
      Assembler begin
         strchar char,retstr,j
         itos char
      assembler end
         if char=schar then hlen=hlen-1
         else do
            retstr=substr(retstr,1,hlen)
            return retstr
         end
      end
   end
return retstr

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '
