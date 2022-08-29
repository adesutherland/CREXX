/*
 * rexx built-in function Datatype
 */
options levelb

namespace rxfnsb

/* call CheckArgs  'rANY oABLMNSUWX' */
/* As well as returning the type, the value for a 'NUM' is set in
   !DatatypeResult.  This is a convenience when DATATYPE is used
   by CHECKARGS. */
datatype: procedure = .string 
  arg expose string_in = .string, Type = ""

say 'start of datatype'
say 'arguments are:' string_in Type
/* TODO: inherit this from some central place */
AllBlanks = ' '
Limit_ExponentDigits = 99
FormLevel = 'ENGINEERING' /* we don't know this yet */

/* If no second argument, DATATYPE checks whether the first is a number. */
argc=0
if Type = "" then argc=1
else argc=2

say 'argc =' argc
if argc = 1 then return DtypeOne(string_in)
 
 /* Null strings are a special case. */
 if string_in = '' then do
   if Type = "X" then return 1
   /* if Type = "B" & \!NotJ18 then return 1 */
   return 0
 end
 
 /* Several of the options are shorthands for VERIFY */
 azl="abcdefghijklmnopqrstuvwxyz"
 AZU="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
 D09="0123456789"
 if Type = "A" then return verify(string_in,azl||AZU||D09)=0
 if Type = "B" then do
 /*   /\* if !NotJ18 then return verify(string_in,"01")=0 *\/ */
   /* Check blanks in allowed places. */
   if pos(left(string_in,1),AllBlanks)>0 then return 0
   if pos(right(string_in,1),AllBlanks)>0 then return 0
   BinaryDigits=0
   do j = length(string_in) by -1 to 1
     c = substr(string_in,j,1)
     if pos(c,AllBlanks)>0 then do
       /* Blanks need four BinaryDigits to the right of them. */
       if BinaryDigits//4 \= 0 then return 0
     end
   else do
     if verify(c,"01") \= 0  then return 0
     BinaryDigits = BinaryDigits + 1
   end
   end
   return 1
 end /* B */
 if Type = "L" then return(verify(string_in,azl)=0)
 if Type = "M" then return(verify(string_in,azl||AZU)=0)
 if Type = "N" then return(datatype(string_in)="NUM")
 /* TODO need Symbol if Type = "S" then return(symbol(string_in)\='BAD') */
 if Type = "U" then return(verify(string_in,AZU)=0)
 if Type = "W" then do
   /* It may not be a number. */
   if DtypeOne(string_in) = 'CHAR' then return '0'
   /* It can be "Whole" even if originally in exponential notation. */
   /* BLM Nov 98 */
   /* Before, After and Exponent are byproducts of DtypeOne. */
   /* if !NotJ18 then do */
   /*   /\* Allow 1.0000000003 if fraction lost by DIGITS setting. *\/ */
   /*   string_in=Plus0(string_in) */
   /*   InFraction='0' */
   /*    do j = 1 to length(string_in) */
   /*      c = substr(string_in,j,1) */
   /*      if pos(c,'Ee')>0 then return 0 */
   /*      if InFraction & pos(c,'+-')>0 then return 0 */
   /*      if c = '.' then InFraction='1' */
   /*      else if InFraction & c\='0' then return 0 */
   /*    end j */
   /*    !DatatypeResult = !DatatypeResult % 1 */
   /*    return 1 */
   /* end */

   /* BLM Jun 99 */
   /* It won't be "Whole" if DtypeOne needed to give it an exponent. */
   /* if pos('E',!DataTypeResult) > 0 then return 0 */
   /* It won't be "Whole" if there is a non-zero after the decimal point. */
   /* This test has to be against the original string, not !DataTypeResult
      which has had zero added. */
      ExpWas = 0 /* TODO: some global var */
      string_in = Before||After
      j = length(Before)+ExpWas
      if j<0 then j = 0
      if verify(substr(string_in,j+1),'0') > 0 then return 0
      /* All tests for Whole passed. */
      /* !DatatypeResult = !DatatypeResult % 1 */
      return 1
 end /* W */
 /* Type will be "X" */
 if pos(left(string_in,1),AllBlanks)>0 then return 0
 if pos(right(string_in,1),AllBlanks)>0 then return 0
 HexDigits=0
 do j=length(string_in) by -1 to 1
   c=substr(string_in,j,1)
   if pos(c,AllBlanks)>0 then do
     /* Blanks need a pair of HexDigits to the right of them. */
     if HexDigits//2 \= 0 then return 0
   end
 else do
   if verify(c,"abcdefABCDEF"D09) \= 0  then return 0
   HexDigits=HexDigits+1
 end
 end
 return 1
 /* end X */
 
 /* Plus0:                   /\* Just for NotJ18 *\/ */
 /* /\* This check is made with the digits setting of the caller. *\/ */
 /* numeric digits !Digits.!Level */
 /* numeric form value !Form.!Level */
 /* return 0+arg(1) */
 
DtypeOne: procedure = .string
 arg string_in = .string
 say 'in DtypeOne'
 say 'arg is' string_in
 /* See section 7 for the syntax of a number. */
 /* !DatatypeResult = 'S' /\* If not syntactically a number *\/ */
 Residue = strip(string_in) /* Blanks are allowed at both ends. */
 say 'Residue is' Residue
 if Residue = '' then return "CHAR"
 Sign = ''
 say 'first if'
 if left(Residue,1) = '+' | left(Residue,1) = '-' then do
   Sign = left(Residue, 1)
   Residue = strip(substr(Residue,2),'L') /* Blanks after sign */
 end
 say 'second if'
 if Residue = '' then return "CHAR"
 /* Now testing Number, section 6.2.2.35 */
 say 'third if'
 if left(Residue,1) = '.' then do
   Residue = substr(Residue, 2)
   Before = ''
   After = DigitRun(Residue)
   if After = '' then return "CHAR"
 end
 else do
   say 'in else'
   After='' /* BLM Nov 98. Needed as byproduct */
   Before = DigitRun(Residue)
   if Before = '' then return "CHAR"
   say 'before left in else'
   if left(Residue,1) = '.' then do
     Residue = substr(Residue, 2)
     After = DigitRun(Residue)
     say 'after left in else'
   end
 end
 Exponent = 0
 if Residue \= '' then do
   if left(Residue, 1) \= 'e' & left(Residue, 1) \= 'E' then
     return "CHAR"
 Residue = substr(Residue, 2)
 if Residue = '' then return "CHAR"
 Esign = ''
 if left(Residue, 1) = '+' | left(Residue, 1) = '-' then do
   Esign = left(Residue, 1)
   Residue = substr(Residue, 2)
   if Residue = '' then return "CHAR"
 end
 Exponent = DigitRun(Residue)
 if Exponent = '' then return "CHAR"
 Exponent = Esign || Exponent
 end
 if Residue \= '' then return "CHAR"
 ExpWas=Exponent /* BLM for Datatype 'W' */
 
 /*DATATYPE tests for exponent out of range. */
 /* !DatatypeResult = 'E' /\* If exponent out of range *\/ */
 Before = strip(Before,'L','0')
 if Before = '' then Before = '0'
 Exponent = Exponent + length(Before) -1   /* For SCIENTIFIC */
 
 /* "Engineering notation causes powers of ten to expressed as a
    multiple of 3 - the integer part may therefore range from 1 through 999." */
    g = 1
    /* if FormLevel = 'E' then do */
    /*   /\* Adjustment to make exponent a multiple of 3 *\/ */
    /*   g = Exponent//3 */
    /*   if g < 0 then g = g + 3 */
    /*   Exponent = Exponent - g */
    /* end */
    
    /* Check on the exponent. */
    g = length(Exponent)
    if Exponent<0 then g=g-1
    if g > Limit_ExponentDigits then return "CHAR"
    
    /* Format to the numeric setting of the caller of DATATYPE */
    /* numeric digits !Digits.!Level */
    /* numeric form value !Form.!Level */
    /* !DatatypeResult = 0 + !Bif_Arg.1 */
    return "NUM"
    
DigitRun: procedure = .string
arg Residue = .string
say 'in DigitRun'
say 'Residue is' Residue
Outcome = ''
do while Residue \= ''
  say 'in do while digitrun'
  if pos(left(Residue, 1), '0123456789') = 0 then leave
  say 'after pos in digitrun'
  Outcome = Outcome || left(Residue, 1)
  say 'after outcome in digitrun'
  say 'Residue after outcome in digitrun' Residue
  Residue = substr(Residue,2)
  say 'Residue in digitrun is' Residue
end
say 'Outcome of digitrun' Outcome
return Outcome
