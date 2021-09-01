 /************************************************************************
 *.  V A L U E   9.8.6
 ************************************************************************/
 if !Bif_ArgExists.3 then ArgData = 'rANY oANY oANY'
 else ArgData = 'rSYM oANY oANY'
 call CheckArgs ArgData
 
 Subject = !Bif_Arg.1
 if !Bif_ArgExists.3 then do  /* An external pool. */
   /* Fetch the original value */
   Pool = !Bif_Arg.3
   Indication = Config_Get(Pool,Subject)
   Indicator = left(Indication,1)
   if Indicator == 'F' then
     call Raise 40.36, Subject
   if Indicator == 'P' then
     call Raise 40.37, Pool
   Value = !Outcome
   if !Bif_ArgExists.2 then do
     /* Set the new value. */
     Indication = Config_Set(Pool,Subject,!Bif_Arg.2)
     if Indicator == 'P' then
       call Raise 40.37, Pool
     if Indicator == 'F' then
       call Raise 40.36, Subject
   end
   /* Return the original value. */
   return Value
 end
 /* Not external */
 Subject = translate(Subject)
 /* See 7.3 */
 p = pos(Subject,'.')
 if p = 0 | p = length(Subject) then do
   /* Not compound */
   Indication = Var_Value(!Pool, Subject, '0')
   Value = !Outcome
   if !Bif_ArgExists.2 then
     Indication = Var_Set(!Pool, Subject, '0', !Bif_Arg.2)
   return Value
 end
 /* Compound */
 Expanded = left(Subject,p-1)  /* The stem */
 do forever
   Start = p+1
   p = pos(Subject,'.',Start)
   if p = 0 then p = length(Subject)
   Item = substr(Subject,Start,p-Start) /* Tail component symbol */
   if Item\=='' then if pos(left(Item,1),'0123456789') = 0 then do
     Indication = Var_Value(!Pool, Item, '0')
     Item = !Outcome
   end
   /* Add tail component. */
   Expanded = Expanded'.'Item
 end
 Indication = Var_Value(!Pool, Expanded, '1')
 Value = !Outcome
 if !Bif_ArgExists.2 then
   Indication = Var_Set(!Pool, Expanded, '1', !Bif_Arg.2)
 return Value
