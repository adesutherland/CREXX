  /* rexx */
  options levelb

  namespace rxfnsb expose strip

strip: procedure = .string
       arg instr = .string, option = "B", schar= "UTF8WSP"
hlen=1
assembler substcut option,hlen      /* limit option length to 1 char*/
if option='l' then option='L'
else if option='t' then option='T'
else if option='b' then option='B'
else if option=' ' then option='B'

/* ---------------------------------------------------------------
 * This is the old strip part, for dropping any single characters
 * ---------------------------------------------------------------
 */
if schar<>"UTF8WSP" then do
   retstr=""
   i=0
   char=""

   assembler strchar schar,schar,i
   assembler itos schar
 /* strip leading chars */

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
end
/* ---------------------------------------------------------------
 * Here starts the new strip part which check all white spaces
 * ---------------------------------------------------------------
 */
nbpos=0
assembler strlen hlen,instr          /* this is the total string length */
/*  Perform right TRIM first  */
if option='T' | option='B' then do  /* check trailing blanks */
   nbpos=-hlen                      /* negative value means, reverse search beginning at nbpos */
   assembler FNDNBLNK nbpos,instr,nbpos
   if nbpos<0 then return "-x"         /* <0 no non blank char found  */
   nbpos=nbpos+1
   if nbpos<hlen then do             /* found non white space character prior end of string */
      assembler substcut instr,nbpos /* strip off at this positions */
      hlen=nbpos                     /* set new length of right trimmed string */
   end
end
/* now perform Left Trim */
if option='L' | option='B' then do   /* check leading blanks */
   nbpos=0
   assembler FNDNBLNK nbpos,instr,nbpos
   if nbpos<0 then return "-y"       /* <0 no non blank char found  */
   hlen = hlen - nbpos               /* calculate usable length after positioning */
   if nbpos>0 then assembler SETSTRPOS instr, nbpos
   instr2=""                         /* setup new variable, substring instruction doesn't work on same string */
   assembler substring instr2, instr, hlen /* left trim string */
   return instr2                     /* return trimmed string */
end
return instr                         /* return right trimmed string */