 /* 9.2.1 Argument checking */
 /* Check arguments. Some further checks will be made in particular built-ins.*/
 /* The argument to CheckArgs is a checklist for the allowable arguments. */
 /* NUM and WHOLE have a side-effect, 'normalizing' the number. */
 /* Calls to raise syntax conditions will not return. */

 CheckList = arg(1)  /* This refers to the argument of CheckArgs. */
 
 /* Move the checklist information from a string to individual variables */
 ArgType. = ''
 ArgPos = 0  /* To count arguments */
 MinArgs  = 0
 do j = 1 to length(CheckList)
   ArgPos = ArgPos+1
   /* Count the required arguments. */
   if substr(CheckList,j,1) == 'r' then MinArgs = MinArgs + 1
   /* Collect type information. */
   do while j < length(CheckList)
     j = j + 1
     t = substr(CheckList,j,1)
     if t==' ' then leave
     ArgType.ArgPos = ArgType.ArgPos || t
   end
   /* A single space delimits parts. */
 end j
 MaxArgs = ArgPos
 
 /* Check the number of arguments to the built-in, in this instance. */
 NumArgs  = !Bif_Arg.0
 if NumArgs < MinArgs then call Raise 40.3, MinArgs
 if NumArgs > MaxArgs then call Raise 40.4, MaxArgs
 
 /* Check the type(s) of the arguments to the built-in. */
 do ArgPos = 1 to NumArgs
   if !Bif_ArgExists.ArgPos then
     call CheckType
   else
     if ArgPos <= MinArgs then call Raise 40.5, ArgPos
 end ArgPos
 
 /* No errors found by CheckArgs. */
 return
 
 CheckType:
 
 Value = !Bif_Arg.ArgPos
 Type  = ArgType.ArgPos
 
 select
   when Type == 'ANY' then nop                         /* Any string */
   
   when Type == 'NUM' then do                          /* Any number */
     /* This check is made with the caller's digits setting. */
     if \Cdatatype(Value, 'N') then
       if !DatatypeResult=='E' then call Raise 40.9, ArgPos, Value
       else call Raise 40.11, ArgPos, Value
       !Bif_Arg.ArgPos=!DatatypeResult /* Update argument copy. */
   end
   
   when Type == 'WHOLE' then do                      /* Whole number */
     /* This check is made with digits setting for the built-in. */
     if \Edatatype(Value,'W') then
       call Raise 40.12, ArgPos, Value
   !Bif_Arg.ArgPos=!DatatypeResult
   end
   
   when Type == 'WHOLE>=0' then do      /* Non-negative whole number */
     if \Edatatype(Value,'W') then
       call Raise 40.12, ArgPos, Value
   if !DatatypeResult < 0 then
     call Raise 40.13, ArgPos, Value
   !Bif_Arg.ArgPos=!DatatypeResult
   end
   
   when Type == 'WHOLE>0' then do           /* Positive whole number */
     if \Edatatype(Value,'W') then
       call Raise 40.12, ArgPos, Value
   if !DatatypeResult <= 0 then
     call Raise 40.14, ArgPos, Value
   !Bif_Arg.ArgPos=!DatatypeResult
   end
   
   when Type == 'WHOLENUM' then do                /* D2 Whole number */
     /* This check is made with digits setting of the caller. */
     if \Cdatatype(Value,'W') then
       call Raise 40.12, ArgPos, Value
   !Bif_Arg.ArgPos=!DatatypeResult
   end
   
   when Type == 'WHOLENUM>=0' then do /* D2 Non-negative whole number */
     if \Cdatatype(Value,'W') then
       call Raise 40.12, ArgPos, Value
   if !DatatypeResult < 0 then
     call Raise 40.13, ArgPos, Value
   !Bif_Arg.ArgPos=!DatatypeResult
   end
   
   when Type == '0_90' then do                          /* Errortext */
     if \Edatatype(Value,'N') then
       call Raise 40.11, ArgPos, Value
   Value=!DatatypeResult
   !Bif_Arg.ArgPos=Value
   Major=Value % 1
   Minor=Value - Major
   if Major < 0 | Major > 90 | Minor > .9 | pos('E',Value)>0 then
     call Raise 40.16, Value  /* ArgPos will be 1 */
   end
   
   when Type == 'PAD' then do    /* Single character, usually a pad. */
     if length(Value) \= 1 then
       call Raise 40.23, ArgPos, Value
   end
   
   when Type == 'HEX' then                     /* Hexadecimal string */
     if \datatype(Value, 'X') then
       call Raise 40.25, Value    /* ArgPos will be 1 */
   
   when Type == 'BIN' then                          /* Binary string */
     if \datatype(Value,'B') then
       call Raise 40.24, Value    /* ArgPos will be 1 */
   
   when Type = 'SYM' then                                 /* Symbol */
     if \datatype(Value, 'S') then
       call Raise 40.26, Value   /* ArgPos will be 1 */
   
   when Type = 'ACEFILNOR' then do                         /* Trace */
     /* Allow leading '?'s */
     Val = strip(Value,'Left','?')
     if pos(translate(left(Val, 1)), 'ACEFILNOR') = 0 then
       call Raise 40.28, ArgPos, Type, Value
   end
   
   otherwise do                                           /* Options */
     /* The checklist item is a list of allowed characters */
     if Value == '' then
       call Raise 40.21, ArgPos
   !Bif_Arg.ArgPos = translate(left(Value, 1))
   if pos(!Bif_Arg.ArgPos, Type) = 0 then
     call Raise 40.28, ArgPos, Type, Value
 end
 
end   /* Select */

return

Cdatatype:
/* This check is made with the digits setting of the caller. */
/* !DatatypeResult will be set by use of datatype() */
numeric digits !Digits.!Level
numeric form value !Form.!Level
return datatype(arg(1), arg(2))

Edatatype:
/* This check is made with digits setting for the particular built-in. */
/* !DatatypeResult will be set by use of datatype() */
numeric digits !Config_Digits.!Bif
numeric form scientific
return datatype(arg(1),arg(2))
