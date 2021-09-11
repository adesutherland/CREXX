 /************************************************************************
 *.  L I N E O U T   9.7.5
 ************************************************************************/
 call CheckArgs 'oANY oANY oWHOLE>0'
 
 if !Bif_ArgExists.1 then Stream  = !Bif_Arg.1
 else Stream  = ''
 if !Bif_ArgExists.3 then do
   Linepos = !Bif_Arg.3
   call Config_Position Stream,'LINEOUT',linepos   /* May fail with 40.41 */
 end
 if \!Bif_ArgExists.2 then do
   if !Bif_ArgExists.3 then return 0
   /* Position to end of stream. */
   do until left(Indication,1) \== 'N'
     Indication = Config_Charin(Stream,'LINEIN')
   end
   return 0
 end
 String = !Bif_Arg.2
 Stride = 1
 Residue = length(String)
 call Config_StreamQuery Stream
 Mode = !Outcome
 if Mode == 'B' then do
   call Config_C2B String
   String = !Outcome
   Stride = 8
   Residue = length(String)/8
 end
 Cursor = 1
 do while Residue > 0
   Piece = substr(String,Cursor,Stride)
   Cursor = Cursor+Stride
   call Config_Charout Stream,Piece
   if !This_Exception \== 'READY' then do
     call !Raise !This_Exception  /* Which may not return */
     return 1
   end
   Residue = Residue-1
 end
 return 0
