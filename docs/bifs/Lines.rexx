 /************************************************************************
 *.  L I N E S    9.7.6
 ************************************************************************/
 call CheckArgs 'oANY oCNI'
 
 if !Bif_ArgExists.1 then Stream = !Bif_Arg.1
 else Stream = ''
 if !Bif_ArgExists.2 then Option = !Bif_Arg.2
 else Option = 'N'
 
 Call Config_StreamCount Stream, 'LINEIN', Option
 return !Outcome
