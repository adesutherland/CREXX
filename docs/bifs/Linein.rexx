 /************************************************************************
 *.  L I N E I N   9.7.4
 ************************************************************************/
 call CheckArgs 'oANY oWHOLE>0 oWHOLE>=0'
 
 if !Bif_ArgExists.1 then Stream = !Bif_Arg.1
 else Stream = ''
 if !Bif_ArgExists.2 then do
   linepos  = !Bif_Arg.2
   /* Already rested for > 0 */
   /* May fail with 40.41 */
   call Config_Position Stream,'LINEIN',linepos
 end
 if !Bif_ArgExists.3 then Count = !Bif_Arg.3
 else Count = 1
 if Count>1 then call Raise 40.42, Count
 if Count = 0 then return ''
 /* A configuration may recognise lines even in 'binary' mode. */
 call Config_StreamQuery Stream
 Mode = !Outcome
 r = ''
 t = !Linein_Position.Stream
 do until t \= !Linein_Position.Stream
   do until left(Indication,1) \== 'T'
     Indication = Config_Charin(Stream,'LINEIN')
   end
   if !This_Exception \== 'READY' then do
     call !Raise !This_Exception  /* Which may not return */
     return ""
   end
   r = r||!Outcome
 end
 if Mode == 'B' then do
   call Config_B2C r
   r = !Outcome
 end
 return r
