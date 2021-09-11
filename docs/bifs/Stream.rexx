 /************************************************************************
 *.  S T R E A M    9.7.8
 ************************************************************************/
 /* Third argument is only correct with 'C' */
 if !Bif_ArgExists.2 & translate(left(!Bif_Arg.2, 1)) == 'C' then
   ArgData = 'rANY rCDS rANY'
 else
       ArgData = 'rANY oCDS'
 call CheckArgs ArgData
 
 Stream = !Bif_Arg.1
 
 if !Bif_ArgExists.2 then Operation = !Bif_Arg.2
 else Operation = 'S'
 
 Select
   when Operation == 'C' then do
     call Config_StreamCommand Stream,!Bif_Arg.3
     return !Outcome
   end
   when Operation == 'D' then do
     call Config_StreamState Stream
     return !Description
   end
   when Operation == 'S' then do
     call Config_StreamState Stream
     return !This_Exception
   end
 end
