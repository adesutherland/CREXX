/* rexx */
options levelb
/* X2b(hex-string)  returns bit combination of hex string */

x2b: procedure = .string
 arg hex = .string, slen=-1

 if hex="" then return ""
/*      0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111 */
bittab="0000000100100011010001010110011110001001101010111100110111101111"
trtab="0123456789ABCDEFabcdef "
char=""
rstr=""
hlen=0
offset=0
added=0
assembler strlen hlen,hex

do i=0 to hlen-1
   assembler strchar char,hex,i
   assembler poschar offset,trtab,char  /* position in hex table   */
   if offset<0 then do
      say "hex string contains invalid character "hex
      return
   end
   if added=0 & offset=0 then iterate  /* ignore first leading zero */
   added=added+1
   if offset=22 then iterate       /* it's a blank ignore it */
   if offset>15 then offset=offset-6
   offset=offset*4
   /* rstr\="" then rstr=rstr||" " */
   do k=offset to offset+3
      assembler concchar rstr,bittab,k
   end
end
if added=0 then rstr='0000'
return rstr