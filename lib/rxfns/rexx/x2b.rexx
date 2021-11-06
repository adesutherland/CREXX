/* rexx */
options levelb
/* X2b(hex-string)  returns bit combination of hex string */

x2b: procedure = .string
 arg hex = .string
bittab="0000000100100011010001010110011110001001101010111100110111101111"
trtab="0123456789ABCDEFabcdef"
char=""
rstr=""
hlen=0
offset=0
assembler strlen hlen,hex
do i=0 to hlen-1
   assembler strchar char,hex,i
   assembler poschar offset,trtab,char  /* position in hex table   */
   if offset<0 then do
      say "hex string contains invalid character "hex
      return
   end
   if rstr\="" then rstr=rstr||" "
   do k=offset to offset+3
      assembler concchar rstr,bittab,k
   end
end
return rstr