 /************************************************************************
 *.  L A S T P O S    9.3.10
 ************************************************************************/
 call CheckArgs 'rANY rANY oWHOLE>0'
 
 Needle   = !Bif_Arg.1
 Haystack = !Bif_Arg.2
 if !Bif_ArgExists.3 then Start = !Bif_Arg.3
 else Start = length(Haystack)
 
 NeedleLength = length(Needle)
 if NeedleLength = 0 then return 0
 Start = Start - NeedleLength + 1
 do i = Start by -1 while i > 0
   if substr(Haystack, i, NeedleLength) == Needle then return i
 end i
 return 0
 