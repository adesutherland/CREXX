 /************************************************************************
 *.  X R A N G E    9.8.28
 ************************************************************************/
 call CheckArgs 'oPAD oPAD'
 
 if \!Bif_ArgExists.1 then !Bif_Arg.1 = ''
 if \!Bif_ArgExists.2 then !Bif_Arg.2 = ''
 Indication = Config_xrange(!Bif_Arg.1, !Bif_Arg.2)
 return !Outcome
