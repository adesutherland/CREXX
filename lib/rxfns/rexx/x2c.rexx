/* rexx */
options levelb
/* X2d(hex-string)  returns decimal number of hex string */

x2c: procedure = .string
  arg hex = .string
  trtab="0123456789ABCDEFabcdef"
  slen=0
  char1=0
  char2=0
  off1=0
  off2=0
  binary=""
  byte=0
  hexi=""
  blank=" "
  assembler strchar char2,blank,byte  /* misuse byte as position 0 */
  assembler strlen slen,hex    /* get length of hex string */
  xlen=slen
  do i=0 to xlen-1
     assembler strchar char1,hex,i
     if char1=char2 then do
        slen=slen-1
        iterate
     end
     assembler concchar hexi,hex,i
  end
  if slen//2=1 then do
     hexi='0'hexi
     slen=slen+1
  end

  do i=0 to slen-1 by 2                   /* loop through hex string */
     assembler strchar char1,hexi,i        /* fetch one byte          */
     assembler poschar off1,trtab,char1   /* position in hex table   */
     j=i+1
     assembler strchar char2,hexi,j        /* fetch one byte          */
     assembler poschar off2,trtab,char2   /* position in hex table   */

     if off1<0 | off2<0 then do           /* no hex char? error!     */
        say "hex string contains invalid character "hexi
        return 0
     end
     if off1>15 then off1=off1-6    /* translate lower case to upper case hex */
     if off2>15 then off2=off2-6    /* translate lower case to upper case hex */
     assembler ishl byte,off1,4     /* move 1. char to left hand part of byte */
     byte=byte+off2                 /* move 2. char to right hand part of byte*/
     assembler appendchar binary,byte /* append to result                     */
  end
 return binary  /* return result */