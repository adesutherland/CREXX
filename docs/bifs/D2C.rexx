 /************************************************************************
 *.  D 2 C   9.6.7
 ************************************************************************/

 if \!Bif_ArgExists.2 then ArgData = 'rWHOLENUM>=0'
 else ArgData = 'rWHOLENUM rWHOLE>=0'
 call CheckArgs ArgData
 
 /* Convert to manifest binary */
 Subject = abs(!Bif_Arg.1)
 r = ReRadix(Subject,10,2)
 /* Make length a multiple of 8, as required for Config_B2C */
 Length = length(r)
 do while Length//8 \= 0
   Length = Length+1
 end
 r = right(r,Length,'0')
 /* 2s-complement for negatives. */
 if !Bif_Arg.1<0 then do
   Subject = 2**length(r)-Subject
   r = ReRadix(Subject,10,2)
 end
 /* Convert to characters */
 Indication = Config_B2C(r)
 Output = !Outcome
 if \!Bif_ArgExists.2 then return Output
 
 /* Adjust the length with appropriate characters. */
 if !Bif_Arg.1>=0 then return right(Output,!Bif_Arg.2,left(xrange(),1))
 else return right(Output,!Bif_Arg.2,right(xrange(),1))

