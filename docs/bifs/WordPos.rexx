 /************************************************************************
 *.  W O R D P O S  9.3.26
 ************************************************************************/
 call CheckArgs 'rANY rANY oWHOLE>0'
 
 Phrase = !Bif_Arg.1
 String = !Bif_Arg.2
 if !Bif_ArgExists.3 then Start = !Bif_Arg.3
 else Start = 1
 
 Phrase = space(Phrase)
 PhraseWords = words(Phrase)
 if PhraseWords = 0 then return 0
 String = space(String)
 StringWords = words(String)
 do WordNumber = Start to StringWords - PhraseWords + 1
   if Phrase == subword(String, WordNumber, PhraseWords) then
     return WordNumber
 end WordNumber
 return 0

