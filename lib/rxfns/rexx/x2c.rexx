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
  assembler strlen slen,hex    /* get length of hex string */

  do i=0 to slen-1 by 2                   /* loop through hex string */
     assembler strchar char1,hex,i        /* fetch one byte          */
     assembler poschar off1,trtab,char1   /* position in hex table   */
     j=i+1
     assembler strchar char2,hex,j        /* fetch one byte          */
     assembler poschar off2,trtab,char2   /* position in hex table   */

     if off1<0 | off2<0 then do           /* no hex char? error!     */
        say "hex string contains invalid character "hex
        return 0
     end
     if off1>15 then off1=off1-6    /* translate lower case to upper case hex */
     if off2>15 then off2=off2-6    /* translate lower case to upper case hex */
     assembler ishl byte,off1,4     /* move 1. char to left hand byte         */
     byte=byte+off2                 /* move 2. char to right hand byte        */
     assembler appendchar binary,byte /* append to result                     */
  end

 return binary  /* return result */