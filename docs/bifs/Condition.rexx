 /************************************************************************
 *.  C O N D I T I O N   9.5.3
 ************************************************************************/
 call CheckArgs 'oCDEIS'
 
 Option=!Bif_Arg.1
 if Option=='C' then return !Condition.!Level
 if Option=='D' then return !ConditionDescription.!Level
 if Option=='E' then return !ConditionExtra.!Level
 if Option=='I' then return !ConditionInstruction.!Level
 /* S */             return !ConditionState.!Level
 
