 /* rexx */
  options levelb

format: procedure = .string
   arg innum = .string, before = 0, after = 0, expp = 0, expt=0

   ilen=0
   formatx=_itrunc(innum)
   formaty=_ftrunc(innum)

   if before>0 then do
      assembler strlen ilen,formatx
      if ilen>before then return 'integer part number to large'
      formatx=right(formatx,before,' ')
   end
   if after>0 then do
      formaty=left(formaty,after,'0')
      formatx=formatx'.'formaty
   end
return formatx


/* Prototype functions */
_itrunc: procedure = .string
       arg innum = .float
_ftrunc: procedure = .string
       arg innum = .float
right: procedure = .string
       arg string = .string, length1 = .int, pad = '0'
left: procedure = .string
       arg string = .string, length1 = .int, pad = '0'



