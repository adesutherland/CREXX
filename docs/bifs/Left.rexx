 /************************************************************************
 *.  L E F T     9.3.11
 ************************************************************************/
 call CheckArgs 'rANY rWHOLE>=0 oPAD'
 
 if !Bif_ArgExists.3 then Pad = !Bif_Arg.3
 else Pad = ' '
 
 return substr(!Bif_Arg.1, 1, !Bif_Arg.2, Pad)
