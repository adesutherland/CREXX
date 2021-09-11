 /************************************************************************
 *.  C O M P A R E  9.3.4
 ************************************************************************/
 call CheckArgs  'rANY rANY oPAD'
 
 Str1 = !Bif_Arg.1
 Str2 = !Bif_Arg.2
 if !Bif_ArgExists.3 then Pad = !Bif_Arg.3
 else Pad = ' '
 
 /* Compare the strings from left to right one character at a time */
 if length(Str1) > length(Str2) then do
   Length = length(Str1)
   Str2=left(Str2,Length,Pad)
 end
 else do
   Length = length(Str2)
   Str1=left(Str1,Length,Pad)
 end
 
 Str1=translate(Str1, ,!AllBlanks,' ')
 Str2=translate(Str2, ,!AllBlanks,' ')
 
 do i = 1 to Length
   if substr(Str1, i, 1) \== substr(Str2, i, 1) then return i
 end
 return 0
