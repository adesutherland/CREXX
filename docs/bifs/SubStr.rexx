 /*************************************************************************
 *.  S U B S T R     9.3.19
 ************************************************************************/
 call CheckArgs 'rANY rWHOLE>0 oWHOLE>=0 oPAD'
 
 String = !Bif_Arg.1
 Num    = !Bif_Arg.2
 if !Bif_ArgExists.3 then Length = !Bif_Arg.3
 else Length = length(String)+1-Num
 if !Bif_ArgExists.4 then Pad = !Bif_Arg.4
 else Pad = ' '
 
 Output = ''
 do Length
   Indication = Config_Substr(String,Num) /* Attempt to fetch character.*/
   Character = !Outcome
   Num = Num + 1
   call Config_Substr Indication,1 /* Was there such a character? */
   if !Outcome == 'E' then do
     /* Here if argument was not a character string. */
     
     call Config_C2B String
     call !Raise 'SYNTAX', 23.1, b2x(!Outcome)
     /* No return to here */
   end
   if !Outcome == 'M' then Character = Pad
   Output=Output||Character
 end
 return Output
