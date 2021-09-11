 /************************************************************************
 *.  C H A R I N   9.7.1
 ************************************************************************/
 call CheckArgs 'oANY oWHOLE>0 oWHOLE>=0'
 
 if !Bif_ArgExists.1 then Stream  = !Bif_Arg.1
 else Stream  = ''
 if !Bif_ArgExists.2 then do
   Charpos = !Bif_Arg.2
   /* Already tested for > 0 */
   /* ??      if Charpos > chars(Stream,'C') then
      Chars w/o close doesn't work on OS/2 anyway
      if Charpos > chars(Stream) then
      call Raise 40.41,!Bif_Arg.2  */
   Indication = Config_Position(Stream,'CHARIN',Charpos)
   if left(Indication,1) == 'B' then call Raise 40.27,1,Stream
 end
 if !Bif_ArgExists.3 then Count = !Bif_Arg.3
 else Count = 1
 if Count = 0 then do
   call Config_Charin Stream, 'NULL' /* "Touch" the stream */
   return ''
 end
 /* The unit may be eight bits or one character. */
 call Config_StreamQuery Stream
 Mode = !Outcome
 r = ''
 do until Count = 0
   call Config_Charin Stream, 'CHARIN'
   if !This_Exception \== 'READY' then do
     call !Raise 'NOTREADY', Stream
     leave
   end
   r = r||!Outcome
   Count = Count-1
 end
 if Mode == 'B' then do
   call Config_B2C r
   r = !Outcome
 end
 return r

