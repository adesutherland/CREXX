 /************************************************************************
 *.  S O U R C E L I N E  9.5.8
 ************************************************************************/
 call CheckArgs 'oWHOLE>0'
 
 if \!Bif_ArgExists.1 then return !Sourceline.0
 Num = !Bif_Arg.1
 if Num > !Sourceline.0 then
   call Raise 40.34, Num, !Sourceline.0
 return !Sourceline.Num
