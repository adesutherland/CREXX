 /************************************************************************
 *.  S I G N    9.4.5
 ************************************************************************/
 call CheckArgs 'rNUM'
 
 Number = !Bif_Arg.1
 
 select
   when Number < 0 then Output = -1
   when Number = 0 then Output =  0
   when Number > 0 then Output =  1
 end
 return Output
