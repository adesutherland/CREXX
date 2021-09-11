 /************************************************************************
 *.  P O S    9.3.14
 ************************************************************************/
 call CheckArgs 'rANY rANY oWHOLE>0'
 
 Needle   = !Bif_Arg.1
 Haystack = !Bif_Arg.2
 if !Bif_ArgExists.3 then Start = !Bif_Arg.3
 else Start = 1
 
 if length(Needle) = 0 then return 0
 do i = Start to length(Haystack)+1-length(Needle)
   if substr(Haystack, i, length(Needle)) == Needle then return i
 end i
 return 0
 