/* rexx */
options levelb

/* delword(string,wordnumber-to-delete,length) delete one word, or the remaining words in string */
delword: procedure = .string
  arg expose string = .string, wnum = .int, wcount = .int
  wrds=words(string)
  if wcount=0 then wcount=wrds

  if wnum>wrds then return string
  if wnum<1    then return string

  wdel=0
  wlen=0
  slen=0

  do dw=1 to 8192                   /* temporary solution, until we have a do forever */
     assembler strlen slen,string   /* length of original string */
     if slen<1 then return ""
     wpos=wordindex(string,wnum)    /* locate position of word x */
     if wpos=0 then return string
     wrd= word(string,wnum)         /* load word      */
     assembler strlen wlen,wrd      /* length of word */
     xpos=wpos+wlen                 /* next position following word */
     if xpos<slen then do           /* if still in source string, check if char is blank */
        if substr(string,xpos,1)=' ' then wlen=wlen+1 /* if so, take it as part of word, increase word length */
     end
     if wpos+wlen>slen then do      /* if next byte after word exceeds length, it's last word */
        if wpos=1 then retstr=''    /* if start position 1: empty string remains              */
        else retstr=substr(string,1,wpos-1)  /* else take string prior to word                */
     end
     else do                        /* next word is within string        */
        if wpos=1 then retstr=substr(string,wpos+wlen)   /* take remain string after word     */
        else do                     /* drop word and re-construct string */
           if wnum<wrds then retstr=substr(string,1,wpos-1)||substr(string,wpos+wlen)  /* drop was in the middle */
           else retstr=substr(string,1,wpos-1)  /* drop was last word  */
        end
     end
     string=retstr         /* move back to original string and loop    */
     wdel=wdel+1           /* increase deleted count                   */
     if wdel>=wcount then return string /* match it with the requested count? Then return */
  end
return string

word: procedure = .string
  arg string1 = .string, wordnum = .int

words: procedure = .int
  arg string1 = .string

wordindex: procedure = .int
  arg string1 = .string, wordnum = .int
  /* Length() Procedure - needed for the substr declaration */
length: procedure = .int
    arg string1 = .string

  /* Substr() Procedure */
substr: procedure = .string
     arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ' '
