 /************************************************************************
 *.  C 2 D   9.6.5
 ************************************************************************/
 call CheckArgs 'rANY oWHOLE>=0'
 
 if length(!Bif_Arg.1)=0 then return 0
 
 if !Bif_ArgExists.2 then do
   /* Size specified */
   Size = !Bif_Arg.2
   if Size = 0 then return 0
   /* Pad will normally be zeros */
   t=right(!Bif_Arg.1,Size,left(xrange(),1))
   /* Convert to manifest bit */
   call Config_C2B t
   /* And then to signed decimal. */
   Sign = Left(!Outcome,1)
   !Outcome = substr(!Outcome,2)
   t=ReRadix(!Outcome,2,10)
   /* Sign indicates 2s-complement. */
   if Sign then t=t-2**length(!Outcome)
   if abs(t) > 10**!Digits.!Level-1 then call Raise 40.35,t
   return t
 end
 /* Size not specified. */
 call Config_C2B !Bif_Arg.1
 t=ReRadix(!Outcome,2,10)
 if t > 10**!Digits.!Level-1 then call Raise 40.35,t
 return t
