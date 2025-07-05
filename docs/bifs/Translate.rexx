 /************************************************************************
 *.  T R A N S L A T E   9.3.21
 ************************************************************************/
 call CheckArgs 'rANY oANY oANY oPAD'
 String = !Bif_Arg.1
 /* If neither input nor output tables, uppercase. */
 if \!Bif_ArgExists.2 & \!Bif_ArgExists.3 then do
   Output = ''
   do j=1 to length(String)
     Indication = Config_Upper(substr(String,j,1))
     Output = Output || !Outcome
   end j
   return Output
 end
 /* The input table defaults to all characters. */
 if \!Bif_ArgExists.3 then do
   Indication = Config_Xrange()
   Tablei = !Outcome
 end
 else Tablei = !Bif_Arg.3
 /* The output table defaults to null */
 if !Bif_ArgExists.2 then Tableo = !Bif_Arg.2
 else Tableo = ''
 /* The tables are made the same length */
 if !Bif_ArgExists.4 then Pad = !Bif_Arg.4
 else Pad = ' '
 Tableo=left(Tableo,length(Tablei),Pad)
 
 Output=''
 do j=1 to length(String)
   c=substr(String,j,1)
   k=pos(c,Tablei)
   if k=0 then Output=Output||c
   else Output=Output||substr(Tableo,k,1)
 end j
 return Output
 