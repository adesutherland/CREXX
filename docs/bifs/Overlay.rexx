 /************************************************************************
 *.  O V E R L A Y   9.3.13
 ************************************************************************/
 call CheckArgs 'rANY rANY oWHOLE>0 oWHOLE>=0 oPAD'
 
 New    = !Bif_Arg.1
 Target = !Bif_Arg.2
 if !Bif_ArgExists.3 then Num = !Bif_Arg.3
 else Num = 1
 if !Bif_ArgExists.4 then Length = !Bif_Arg.4
 else Length = length(New)
 if !Bif_ArgExists.5 then Pad = !Bif_Arg.5
 else Pad = ' '
 
 return left(Target, Num - 1, Pad),    /* To left of overlay  */
 || left(New, Length, Pad),         /* New string overlaid */
 || substr(Target, Num + Length)    /* To right of overlay */
