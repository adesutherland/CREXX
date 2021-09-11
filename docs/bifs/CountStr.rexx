 /************************************************************************
 *.  C O U N T S T R
 ************************************************************************/
 call CheckArgs   'rANY rANY'
 
 Output = 0
 Position = pos(!Bif_Arg.1,!Bif_Arg.2)
 do while Position > 0
   Output = Output + 1
   Position = pos(!Bif_Arg.1, !Bif_Arg.2, Position + length(!Bif_Arg.1))
 end
 return Output

