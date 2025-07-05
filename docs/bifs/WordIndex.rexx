 /************************************************************************
 *.  W O R D I N D E X    9.3.24
 ************************************************************************/
 call CheckArgs 'rANY rWHOLE>0'
 
 String = !Bif_Arg.1
 Num    = !Bif_Arg.2
 
 /* Find starting position */
 Start = 1
 Count = 0
 do forever
   Start = verify(String, !AllBlanks, 'N', Start)  /* Find non-blank */
   if Start = 0 then return 0                   /* Start is beyond end */
   Count = Count + 1                                /* Words found */
   if Count = Num then leave
   Start = verify(String, !AllBlanks, 'M', Start + 1) /* Find blank  */
   if Start = 0 then return 0                 /* Start is beyond end */
 end
 return Start
 