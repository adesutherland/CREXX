 /************************************************************************
 *.  V E R I F Y    9.3.22
 ************************************************************************/
 call CheckArgs 'rANY rANY oMN oWHOLE>0'
 
 String    = !Bif_Arg.1
 Reference = !Bif_Arg.2
 if !Bif_ArgExists.3 then Option = !Bif_Arg.3
 else Option = 'N'
 if !Bif_ArgExists.4 then Start = !Bif_Arg.4
 else Start = 1
 
 Last = length(String)
 if Start > Last then return 0
 if Reference == '' then
   if Option == 'N' then return Start
   else return 0
   
   do i = Start to Last
     t = pos(substr(String, i, 1), Reference)
     if Option == 'N' then do
       if t = 0 then return i  /* Return position of NoMatch character. */
     end
   else
     if t > 0 then return i  /* Return position of Matched character. */
   end i
   return 0
