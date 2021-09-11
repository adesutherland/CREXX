 /************************************************************************
 *.  T I M E  9.8.5
 ************************************************************************/
 call CheckArgs 'oCEHLMNORS oANY oCHLMNS'
 /* If the third argument is given then the second is mandatory. */
 if !Bif_ArgExists.3 & \!Bif_ArgExists.2 then
   call Raise 40.19, '', !Bif_Arg.3
 
 if !Bif_ArgExists.1 then Option = !Bif_Arg.1
 else Option = 'N'
 
 /* The date/time is 'frozen' throughout a clause. */
 if !ClauseTime.!Level == '' then do
   !Response = Config_Time()
   !ClauseTime.!Level = !Time
   !ClauseLocal.!Level = !Time + !Adjust
 end
 
 /* If there is no second argument, the current time is returned. */
 if \!Bif_ArgExists.2 then
   return TimeFormat(!ClauseLocal.!Level, Option)
 
 /* If there is a second argument it provides the time to be
    converted. */
 if pos(Option, 'ERO') > 0 then
   call Raise 40.29, Option
 InValue = !Bif_Arg.2
 if !Bif_ArgExists.3 then InOption = !Bif_Arg.3
 else InOption = 'N'
 HH = 0
 MM = 0
 SS = 0
 HourAdjust = 0
 select
   when InOption == 'C' then do
     parse var InValue HH ':' . +1 MM +2 XX
     if HH=12 then HH=0;
     if XX == 'pm' then HourAdjust = 12
   end
   when InOption == 'H' then HH = InValue
   when InOption == 'L' | InOption == 'N' then
     parse var InValue HH ':' MM ':' SS
   when InOption == 'M' then MM = InValue
   otherwise SS = InValue
 end
 if \datatype(HH,'W') | \datatype(MM,'W') | \datatype(SS,'N') then
   call Raise 40.19, InValue, InOption
 HH = HH + HourAdjust
 /* Convert to microseconds */
 Micro = trunc((((HH * 60) + MM) * 60 + SS) * 1000000)
 /* Reconvert to check the original. (eg for hour = 99) */
 if TimeFormat(Micro,InOption) \== InValue then
   call Raise 40.19, InValue, InOption
 return TimeFormat(Micro, Option)
 
 TimeFormat:
 /* Convert from microseconds to given format. */
 parse value Time2Date(arg(1)) with,
 Year Month Day Hour Minute Second Microsecond Base Days
 select
   when arg(2) == 'C' then
     select
       when Hour>12 then
         return Hour-12':'right(Minute,2,'0')'pm'
       when Hour=12 then
         return '12:'right(Minute,2,'0')'pm'
       when Hour>0 then
         return Hour':'right(Minute,2,'0')'am'
       when Hour=0 then
         return '12:'right(Minute,2,'0')'am'
     end
       when arg(2) == 'E' | arg(2) == 'R' then do
         /* Special case first time */
         if !StartTime.!Level == '' then do
           !StartTime.!Level = !ClauseTime.!Level
           return '0'
         end
       Output = !ClauseTime.!Level-!StartTime.!Level
       if arg(2) == 'R' then
         !StartTime.!Level = !ClauseTime.!Level
       return Output * 1E-6
       end  /* E or R */
       when arg(2) == 'H' then return Hour
       when arg(2) == 'L' then
         return right(Hour,2,'0')':'right(Minute,2,'0')':'right(Second,2,'0'),
       || '.'right(Microsecond,6,'0')
       when arg(2) == 'M' then return 60*Hour+Minute
       when arg(2) == 'N' then
         return right(Hour,2,'0')':'right(Minute,2,'0')':'right(Second,2,'0')
       when arg(2) == 'O' then
         return trunc(!ClauseLocal.!Level - !ClauseTime.!Level)
       otherwise /* arg(2) == 'S' */
         return 3600*Hour+60*Minute+Second
 end
