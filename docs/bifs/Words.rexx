 /************************************************************************
 *.  W O R D S    9.3.27
 ************************************************************************/
 call CheckArgs 'rANY'
 
 do Count = 0 by 1
   if subword(!Bif_Arg.1, Count + 1) == '' then return Count
 end Count