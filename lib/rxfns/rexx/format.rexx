 /* rexx */
  options levelb

format: procedure = .string
   arg innum = .string, before = 0, after = 0, expp = 0, expt=-1


   if expt<0 then do  /* format numbers without exponent */
      ilen=0
      formatx=_itrunc(innum)
      formaty=_ftrunc(innum)
      if before>0 then do
         assembler strlen ilen,formatx
         if ilen>before then return 'integer part number to large'
         formatx=right(formatx,before,' ')
      end
      if after>0 then formaty=left(formaty,after,'0')
      return formatx'.'formaty
   end
   else do /* expt>=0 format numbers with exponent */
      rs1=""
      float=0.0
      float=float+innum
      f1="%1.14E"
      assembler fformat rs1,float,f1
      if substr(rs1,2,1)='.' then dp=2
         else dp=3
      formatx=substr(rs1,1,dp-1)
      formaty=substr(rs1,dp+1,14)
      formatz=substr(rs1,dp+17)
      sign=substr(rs1,dp+16,1)
      if before>0 then formatx=right(formatx,before,' ')
      if after=0 then after=3
      formaty=left(formaty,after,'0')
      formatx=formatx'.'formaty
      if expp=0 then expp=1
      formatz1=formatz
      formatz=right(formatz,expp,'0')
      formatz2=formatz
      assembler stoi formatz1
      assembler stoi formatz2
      formatz1=formatz1+0   /* complete conversion (else string content appears) */
      formatz2=formatz2+0
      if formatz1>formatz2 then return "Exponent exceeds maximum formatted Exponent"
      return formatx"E"||sign||formatz
   end
return ''



/* Prototype functions */
_itrunc: procedure = .string
       arg innum = .float
_ftrunc: procedure = .string
       arg innum = .float
right: procedure = .string
       arg string = .string, length1 = .int, pad = '0'
left: procedure = .string
       arg string = .string, length1 = .int, pad = '0'
length: procedure = .int
  arg string1 = .string
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '



