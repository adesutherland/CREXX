 /************************************************************************
 *.  S U B W O R D  9.3.20
 ************************************************************************/
 call CheckArgs 'rANY rWHOLE>0 oWHOLE>=0'
 
 String = !Bif_Arg.1
 Num    = !Bif_Arg.2
 if !Bif_ArgExists.3 then Length = !Bif_Arg.3
 else Length = length(String) /* Avoids call  */
 /*  to WORDS() */
 if Length = 0 then return ''
 /* Find position of first included word */
 Start = wordindex(String,Num)
 if Start = 0 then return ''                   /* Start is beyond end */
 
 /* Find position of first excluded word */
End = wordindex(String,Num+Length)
if End = 0 then End = length(String)+1

Output=substr(String,Start,End-Start)

/* Drop trailing blanks */
do while Output \== ''
  if pos(right(Output,1),!AllBlanks) = 0 then leave
  Output = left(Output,length(Output)-1)
end
return Output
