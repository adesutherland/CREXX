/* rexx */
options levelb
/* X2d(hex-string)  returns decimal number of hex string */

x2d: procedure = .int
  arg hex = .string, slen=-1
  trtab="0123456789ABCDEFabcdef"
  hlen=0
  char=0
  trc=0
  dec=0
  sign=0
  if slen=0 then return 0      /* no hex input return 0    */
  assembler strlen hlen,hex    /* get length of hex string */
  if slen>0 then hex=right(hex,slen,"0")  /* if length parm set, treat hex as signed integer */
  else slen=hlen                          /* else it is an unsigned integer */
  do i=0 to slen-1                     /* loop through hex string */
     assembler strchar char,hex,i      /* fetch one byte          */

     offset=-1
     do j=0 to 21
        assembler strchar trc,trtab,j
        assembler itos trc
        if trc=char then do
           if j>15 then offset=j-6
              else offset=j
           /* leave */
        end
     end
     if offset<0 then do
        say "hex string contains invalid character "hex
        return 0
     end
     char=offset
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