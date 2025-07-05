 /************************************************************************
 *.  E R R O R T E X T    9.5.5
 ************************************************************************/
 call CheckArgs 'r0_90 oSN'
 
 msgcode = !Bif_Arg.1
 if !Bif_ArgExists.2 then Option = !Bif_Arg.2
 else Option = 'N'
 if Option=='S' then return !ErrorText.msgcode
 return !ErrorText.msgcode
