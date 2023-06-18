/* rexx */
options levelb

namespace rxfnsb expose fnv

fnv: procedure = .int
arg input = .string

/* algorithm fnv-1 is */

   hashr = 2166136261   /* 32 bit FNV offset basis value */
   fnvp =   16777619    /* 32 bit FNV prime value */
   data=""
   slen = 0

   assembler strlen slen,input
   do i=0 to slen-1
      assembler strchar data,input,i
      hashr = hashr*FNVP
      assembler ixor hashr,hashr,data
   end
return hashr