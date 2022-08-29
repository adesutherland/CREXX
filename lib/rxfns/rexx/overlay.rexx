/* rexx */
options levelb

namespace rxfnsb

/* overlay(insstr,string,position,length,pad) overlays string into existing string at certain position and length */
overlay: procedure = .string
  arg insstr = .string, string = .string, position = .int, len = 0, pad = ""

  padlen=length(pad)
  if padlen=0 then pad=" " /* define default pad char, just in case we need one */

  slen=length(string)      /* source string */
  if slen=0 then do
     string=pad
     slen=1
  end

  if length(insstr) =0 then do         /* insert string */
     if padlen=0 then return string
     insstr=pad
  end

  if len>0 then do                           /* was there an insert string length?      */
     if padlen>1 then pad=substr(pad,1,1)
     newins=substr(insstr,1,len,pad)         /* format insert string to requested length*/
     insstr=newins
   end
   ilen=length(insstr)

   if position+ilen>slen then string=substr(string,1,position+ilen,pad)  /* is position>string length , extend string */

  if position=1 then str1=''                /* if insert position=1 then str1 is empty */
     else str1=substr(string,1,position-1)  /*    else split string at insert position */
  if position+ilen>slen then str2=''             /* if string was extended str2 is empty    */
     else str2=substr(string,position+ilen) /*    else create str2                     */

 return str1||insstr||str2                   /* return newly constructed string         */
