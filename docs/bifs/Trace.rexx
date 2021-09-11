 /************************************************************************
 *.  T R A C E   9.5.9
 ************************************************************************/
 call CheckArgs 'oACEFILNOR'      /* Also checks for '?' */
 
 /* With no argument, this a simple query. */
 Output=!Tracing.!Level
 if !Interacting.!Level then Output='?'||Output
 if \!Bif_ArgExists.1 then return Output
 
 Value=!Bif_Arg.1
 if Value=='' then !Interacting.!Level='0'
 
 /* Each question mark toggles the interacting. */
 do while left(Value,1)=='?'
   !Interacting.!Level = \!Interacting.!Level
   Value=substr(Value,2)
 end
 /* The default setting is 'Normal' */
 if Value=='' then Value='N'
 Value=translate(left(Value,1))
 if Value=='O' then !Interacting.!Level='0'
 !Tracing.!Level = Value
 return Output
