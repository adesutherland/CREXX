 /************************************************************************
 *.  C 2 X   9.6.6
 ************************************************************************/
 call CheckArgs 'rANY'
 
 if length(!Bif_Arg.1) = 0 then return ''
 call Config_C2B !Bif_Arg.1
 return ReRadix(!Outcome,2,16)

