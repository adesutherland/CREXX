 /************************************************************************
 *.  A D D R E S S   9.5.1
 ************************************************************************/

 /* No second option allowed if first is 'N' or equivalent, otherwise
    second option is required.
    if \!Bif_ArgExists.1 | translate(left(!Bif_Arg.1,1)) == 'N' then
    call CheckArgs  'oN'
    else
    call CheckArgs  'rEIO rANPT'
  */
 
 if !Bif_ArgExists.1 then Option1 = !Bif_Arg.1
 else Option1='N'
 
 if Option1 == 'N' then return !Env_Name.ACTIVE.!Level
 
 Tail = 'ACTIVE.'Option1'.'!Level
 select
   when !Bif_Arg.2='T' then Info = !Env_Type.Tail
   when !Bif_Arg.2='P' then Info = !Env_Position.Tail
   when !Bif_Arg.2='N' then Info = !Env_Resource.Tail
   when !Bif_Arg.2='A' then
     Info = !Env_Type.Tail !Env_Position.Tail !Env_Resource.Tail
 end
 return Info

