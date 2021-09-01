 /************************************************************************
 *.  A B S  9.4.1
 ************************************************************************/
 call CheckArgs  'rNUM'
 
 Number=!Bif_Arg.1
 if left(Number,1) = '-' then Number = substr(Number,2)
 return Number

