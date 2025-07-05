 /************************************************************************
 *.  X 2 D   9.6.11
 ************************************************************************/
 call CheckArgs 'rHEX oWHOLE>=0'
 Subject = !Bif_Arg.1
 if Subject == '' then return '0'
 
 Subject = translate(space(Subject,0))
 /* We put a MIN in here as a practical consideration. Not for Standard. */
 if !Bif_ArgExists.2 then
   Subject = right(Subject,min(!Bif_Arg.2,length(Subject)+1),'0')
 if Subject =='' then return '0'
 /* Note the sign */
 if !Bif_ArgExists.2 then SignBit = left(x2b(Subject),1)
 else SignBit = '0'
 /* Convert to decimal */
 r = ReRadix(Subject,16,10)
 /* Twos-complement */
 if SignBit then r = r - 2**(4*!Bif_Arg.2)
 if abs(r)>10**!Digits.!Level-1 then call Raise 40.35,t
 return r
