 /************************************************************************
 *.  C H A R S  9.7.3
 ************************************************************************/
 call CheckArgs 'oANY oNIC'
 
 if !Bif_ArgExists.1 then Stream = !Bif_Arg.1
 else Stream = ''
 if !Bif_ArgExists.2 then Option = !Bif_Arg.2
 else Option = 'N'
 
 Call Config_StreamCount Stream, 'CHARIN', Option
 return !Outcome
