 /************************************************************************
 *.  W O R D L E N G T H   9.3.25
 ************************************************************************/
 call CheckArgs 'rANY rWHOLE>0'

 return length(subword(!Bif_Arg.1, !Bif_Arg.2, 1))
