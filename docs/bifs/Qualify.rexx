 /************************************************************************
 *.  Q U A L I F Y   9.7.7
 ************************************************************************/
 call CheckArgs 'oANY'
 
 if !Bif_ArgExists.1 then Stream  = !Bif_Arg.1
 else Stream  = ''
 
 Indication = Config_Qualified_Name(Stream)
 return !Outcome
