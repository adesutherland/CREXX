 /* rexx */
  options levelb

namespace rxfnsb expose format
import _rxsysb

format: procedure = .string
   arg innum = .string, before = 0, after = 0, expp = 0, expt=-1
/* --------------------------------------------------------------------------------------
 * Formatting without Exponent
 * --------------------------------------------------------------------------------------
 */
   if expt<0 then do  /* format numbers without exponent */
      ilen=0
      formatx=_itrunc(innum)   /* integer part of number */
      formaty=_ftrunc(innum)   /* fraction part of number*/
      if before>0 then do      /* formatting of integer? */
         assembler strlen ilen,formatx  /* get length    */
         if ilen>before then return 'integer part number to large'
         formatx=right(formatx,before,' ')
      end
      if after>0 then formaty=left(formaty,after,'0') /* formatting of fraction? */
      return formatx'.'formaty
   end
/* --------------------------------------------------------------------------------------
 * Formatting with Exponent
 * --------------------------------------------------------------------------------------
 */
   else do /* expt>=0 format numbers with exponent */
      rs1=""
      float=0.0+innum    /* force input into float */
      f1="%1.14E"        /* format it as exponential with 14 fraction digits */
      assembler fformat rs1,float,f1  /* perform conversion */
      if substr(rs1,2,1)='.' then dp=2  /* seek decimal point, either 2 or 3 */
         else dp=3                 /* format:  +9.12345678901234E+123 */
      formatx=substr(rs1,1,dp-1)   /* select integer part      */
      formaty=substr(rs1,dp+1,14)  /* select fractional part   */
      formatz=substr(rs1,dp+17)    /* select exponent          */
      sign=substr(rs1,dp+16,1)     /* sign of  exponent        */
      if before>0 then formatx=right(formatx,before,' ')
      if after=0 then after=3      /* default precision is 3   */
      formaty=left(formaty,after,'0') /* format fractional part*/
      formatx=formatx'.'formaty    /* concatenate  both        */
      if expp=0 then expp=1        /* default exponent length 1*/
      formatz1=formatz             /* check if exponent fits   */
      formatz=right(formatz,expp,'0') /* format exponent       */
      formatz2=formatz             /* recheck exponent         */
      assembler stoi formatz1      /* convert to integer       */
      assembler stoi formatz2      /* convert to integer       */
      formatz1=formatz1+0   /* complete conversion (else string content appears) */
      formatz2=formatz2+0
      if formatz1>formatz2 then return "Exponent exceeds maximum formatted Exponent"
      return formatx"E"||sign||formatz /* return result          */
   end
return ''

