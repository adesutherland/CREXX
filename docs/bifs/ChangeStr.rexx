 /************************************************************************
 *.  C H A N G E S T R
 ************************************************************************/
 call CheckArgs   'rANY rANY rANY'
 
 Output = ''
 Position = 1
 do forever
   FoundPos = pos(!Bif_Arg.1, !Bif_Arg.2, Position)
   if FoundPos = 0 then leave
   Output = Output || substr(!Bif_Arg.2, Position, FoundPos - Position),
   || !Bif_Arg.3
   Position = FoundPos + length(!Bif_Arg.1)
 end
 return Output || substr(!Bif_Arg.2, Position)

