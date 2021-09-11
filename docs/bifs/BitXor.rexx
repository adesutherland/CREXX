 /************************************************************************
 *.  B I T X O R    9.6.4
 ************************************************************************/
 call CheckArgs  'rANY oANY oPAD'
 
 String1 = !Bif_Arg.1
 if !Bif_ArgExists.2 then String2 = !Bif_Arg.2
 else String2 = ''
 
 /* Presence of a pad implies character strings. */
 if !Bif_ArgExists.3 then
   if length(String1) > length(String2) then
     String2=left(String2,length(String1),!Bif_Arg.3)
   else
     String1=left(String1,length(String2),!Bif_Arg.3)
 
 /* Change to manifest bit representation. */
 Indication=Config_C2B(String1)
 String1=!Outcome
 Indication=Config_C2B(String2)
 String2=!Outcome
 /* Exchange if necessary to make shorter second. */
 if length(String1)<length(String2) then do
   t=String1
   String1=String2
   String2=t
 end
 
 /* Operate on common length of those bit strings. */
 r=''
 do j=1 to length(String2)
   b1=substr(String1,j,1)
   b2=substr(String2,j,1)
   select
     when !Bif='BITAND' then
       b1=b1&b2
     when !Bif='BITOR' then
       b1=b1|b2
     when !Bif='BITXOR' then
       b1=b1&&b2
   end
   r=r||b1
 end j
 r=r || right(String1,length(String1)-length(String2))
 
 /* Convert back to encoded characters. */
 return x2c(b2x(r))

