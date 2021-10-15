/* rexx */
options levelb
/* X2d(hex-string)  returns decimal number of hex string */

x2d: procedure = .int
  arg hex = .string, slen=-1
  hlen=0
  char=0
  dec=0
  sign=0
  if slen=0 then return 0      /* no hex input return 0    */
  assembler strlen hlen,hex    /* get length of hex string */
  if slen>0 then hex=right(hex,slen,'0')  /* if length parm set, treat hex as signed integer */
  else slen=hlen                          /* else it is an unsignded integer

  do i=0 to slen-1                     /* loop through hex string */
     assembler strchar char,hex,i      /* fetch one byte          */
     if char>=97 & char<=102 then do   /* treat a to f has hex numbers */
        char=char-87
     end
     else do                           /* else treat A to F has hex numbers */
       if char>=65 & char <=70 then do
          char=char-55
       end
       else do
         if char>=48 & char <=57 then do /* else translate num chars     */
            char=char-48
         end
         else do                        /* else hex string contains invalid chars */
            say 'invalid hex code 'char
            return -1
         end
       end
     end
  /* if it is first byte then check if sign is set, and convert number */
     if i=0 then do
        if char>=8 & slen>0 then do /* does it contain sign? */
           sign=1           /* keep sign       */
           char=char-8      /* switch off sign */
        end
     end

     dec=dec*16+char        /* cumulate fetched char in result integer */
  end

/* hex string has been processed */
  if sign=1 then do        /* was sign set? */
     char=8                /* build complement */
     do i=1 to slen-1
        char=char*16
     end
     dec=dec-char          /* create integer out of complement */
  end
return dec

/* Right() Procedure */
   right: procedure = .string
          arg string = .string, length1 = .int, pad = '0'