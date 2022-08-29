/* rexx */
options levelb

namespace rxfnsb

/* insert(insstr,string,position,length,pad) inserts string into existing string at certain position and length */
insert: procedure = .string
  arg insstr = .string, string = .string, position = -1, len = -1, pad = " "
/* len=-1 or pos -1 mean, the parameters are not set in the function call */
  slen=length(string)
  if pad='' then pad=' '

  if len>0 then insstr=substr(insstr,1,len,pad) /* was there an insert string length?      */
                                                /* format insert string to requested length*/
  else if len=0 then insstr=""
  if position<=0 then return insstr||string

  if position>slen then string=substr(string,1,position,pad)  /* is position>string length, extend string */

  str1=substr(string,1,position)             /* extract first part of string       */
  if position>=slen then str2=''             /* nothing remains of string          */
  else str2=substr(string,position+1)        /*    else create str2                */
 return str1||insstr||str2                   /* return newly constructed string    */
