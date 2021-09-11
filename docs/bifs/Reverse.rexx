/************************************************************************
 *.  R E V E R S E    9.3.15
 ************************************************************************/
call CheckArgs 'rANY'

String = !Bif_Arg.1

Output = ''
do i = 1 to length(String)
  Output = substr(String,i,1) || Output
end
return Output

