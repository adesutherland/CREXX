/* rexx */
options levelb
/* overlay(insstr,string,position,length,pad) overlays string into existing string at certain position and length */
overlay: procedure = .string
  arg insstr = .string, string = .string, position = .int, len = 0, pad = " "
  slen=length(string)
  if len>0 then do                           /* was there an insert string length?      */
     pad=substr(pad,1,1)
     newins=substr(insstr,1,len,pad)         /* format insert string to requested length*/
     insstr=newins
   end
  ilen=length(insstr)
     say "INSTR '"insstr"'" len ilen
  if len>ilen then ilen=len
  if position+ilen>slen then string=substr(string,1,position+ilen,' ')  /* is position>string length , extend string */

  if position=1 then str1=''                /* if insert position=1 then str1 is empty */
     else str1=substr(string,1,position-1)  /*    else split string at insert position */
  if position+ilen>slen then str2=''             /* if string was extended str2 is empty    */
     else str2=substr(string,position+ilen) /*    else create str2                     */
return str1||insstr||str2                   /* return newly constructed string         */

/* Length() Procedure - needed for the substr declaration */
length: procedure = .int
  arg string1 = .string

/* Substr() Procedure */
substr: procedure = .string
   arg string1 = .string, start = .int, length1 = length(string1) + 1 - start, pad = ''