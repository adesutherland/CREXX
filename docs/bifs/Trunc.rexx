 /************************************************************************
 *.  T R U N C   9.4.6
 ************************************************************************/
 call CheckArgs 'rNUM oWHOLE>=0'
 
 Number = !Bif_Arg.1
 if !Bif_ArgExists.2 then Num=!Bif_Arg.2
 else Num=0
 
 Integer =(10**Num  * Number)%1
 if Num=0 then return Integer
 
 t=length(Integer)-Num
 if t<=0 then return '0.'right(Integer,Num,'0')
 return insert('.',Integer,t)
