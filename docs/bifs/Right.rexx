 /************************************************************************
 *.  R I G H T      9.3.16
 ************************************************************************/
 call CheckArgs 'rANY rWHOLE>=0 oPAD'
 
 String = !Bif_Arg.1
 Length = !Bif_Arg.2
 if !Bif_ArgExists.3 then Pad = !Bif_Arg.3
 else Pad = ' '
 
 Trim = length(String) - Length
 if Trim >= 0 then return substr(String,Trim + 1)
 return copies(Pad, -Trim) || String    /* Pad string on the left */
