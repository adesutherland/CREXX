 /************************************************************************
 *.  S P A C E   9.3.17
 ************************************************************************/
 call CheckArgs 'rANY oWHOLE>=0 oPAD'
 
 String = !Bif_Arg.1
 if !Bif_ArgExists.2 then Num = !Bif_Arg.2
 else Num = 1
 if !Bif_ArgExists.3 then Pad  = !Bif_Arg.3
 else Pad = ' '
 
 Padding = copies(Pad, Num)
 Output = subword(String, 1, 1)
 do i = 2 to words(String)
   Output = Output || Padding || subword(String, i, 1)
 end
 return Output
