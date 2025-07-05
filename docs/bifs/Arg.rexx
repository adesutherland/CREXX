 /************************************************************************
 *.  A R G  9.5.2
 ************************************************************************/
 ArgData = 'oWHOLE>0 oENO'
 if !Bif_ArgExists.2 then ArgData = 'rWHOLE>0 rENO'
 call CheckArgs ArgData
 
 if \!Bif_ArgExists.1 then return !Arg.!Level.0
 
 ArgNum=!Bif_Arg.1
 if \!Bif_ArgExists.2 then return !Arg.!Level.ArgNum
 if !Bif_Arg.2 =='O' then return \!ArgExists.!Level.ArgNum
 else return !ArgExists.!Level.ArgNum
