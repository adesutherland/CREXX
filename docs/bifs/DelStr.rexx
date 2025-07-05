 /************************************************************************
 *.  D E L S T R  9.3.7
 ************************************************************************/
 call CheckArgs  'rANY rWHOLE>0 oWHOLE>=0'
 
 String = !Bif_Arg.1
 Num    = !Bif_Arg.2
 if !Bif_ArgExists.3 then Len = !Bif_Arg.3
 
 if Num > length(String) then return String
 
 Output = substr(String, 1, Num - 1)
 if !Bif_ArgExists.3 then
   if Num + Len <= length(String) then
     Output = Output || substr(String, Num + Len)
 return Output


  