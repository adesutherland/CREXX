 /************************************************************************
 *.  S T R I P   9.3.18
 ************************************************************************/
 call CheckArgs 'rANY oLTB oPAD'
 
 String = !Bif_Arg.1
 if !Bif_ArgExists.2 then Option = !Bif_Arg.2
 else Option = 'B'
 if !Bif_ArgExists.3 then Unwanted = !Bif_Arg.3
 else Unwanted = !AllBlanks
 
 if Option == 'L' | Option == 'B' then do
   /* Strip leading characters */
   do while String \== '' & pos(left(String, 1), Unwanted) > 0
     String = substr(String, 2)
   end
 end
 
 if Option == 'T' | Option == 'B' then do
   /* Strip trailing characters */
   do while String \== '' & pos(right(String, 1), Unwanted) > 0
     String = left(String, length(String)-1)
   end
 end
 return String
