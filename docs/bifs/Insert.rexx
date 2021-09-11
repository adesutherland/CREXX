 /************************************************************************
 *.  I N S E R T    9.3.9
 ************************************************************************/
 call CheckArgs 'rANY rANY oWHOLE>=0 oWHOLE>=0 oPAD'
 
 New    = !Bif_Arg.1
 Target = !Bif_Arg.2
 if !Bif_ArgExists.3 then Num = !Bif_Arg.3
 else Num = 0
 if !Bif_ArgExists.4 then Length = !Bif_Arg.4
 else Length = length(New)
 if !Bif_ArgExists.5 then Pad = !Bif_Arg.5
 else Pad = ' '
 
 return left(Target, Num, Pad),        /* To left of insert   */
 || left(New, Length, Pad),         /* New string inserted */
 || substr(Target, Num + 1)         /* To right of insert  */

