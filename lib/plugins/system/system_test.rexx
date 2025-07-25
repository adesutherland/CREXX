/* REXX */
/* ----------------------------------------------------------------------
 * PRE Compiled on 8 Jul 2025  at 13:21:26
 * ----------------------------------------------------------------------
 */
options levelb
import system
import rxfnsb
##mainargs(allin)
/* ##cflags def nset niflink n1buf n2buf n3buf nvars nmaclist includes  /* set early stage compiler flags */ */
/* ##define strlen(len,string)                   {len=0;         assembler strlen len,string} D*/
myString='C:\Users\PeterJ\CLionProjects\CREXX\250623\cmake-build-debug\lib\plugins\system\abc'
say "'"substr("foobar",8,3)"'"   ## \=   "   "    then
exit
result=''
position=8
mlen=10
count=1000000
start=time('us')
do i=1 to count
   result=substro(myString,position)
end
elp=(time('us')-start)/1000000
say "elapsed  "elp
say 'old SUBSTR "'result'"'
start=time('us')
do i=1 to count
   result=substr(myString,position)
end
elp=(time('us')-start)/1000000
say 'new SUBSTR "'result'"'
say "elapsed    "elp
exit
/*
say ' current c2x presentation, do not handle UTF8 > 1 byte'
say copies('-',100)
say "René:       "c2x("René")
say "Rene:       "c2x("Rene")
say "Adrian:     "c2x("Adrian")
say "Peter:      "c2x("Peter")
say "★ 3 bytes:  "c2x("★")
say "😀 4 bytes: "c2x("😀")
say ' '
say ' C2X with partial UTF8 support, add bytes depending on UTF8 length, not backward compatible'
say copies('-',100)
say "René:       "c2xV3("René")
say "Rene:       "c2xV3("Rene")
say "Adrian:     "c2xV3("Adrian")
say "Peter:      "c2xV3("Peter")
say "★ 3 bytes:  "c2xV3("★")
say "😀 4 bytes: "c2xV3("😀")
say ' '
say ' C2X with partial UTF8 support, add bytes depending on UTF8 length blank separated, not backward compatible'
say copies('-',100)
say "René:       "c2xV4("René")
say "Rene:       "c2xV4("Rene")
say "Adrian:     "c2xV4("Adrian")
say "Peter:      "c2xV4("Peter")
say "★ 3 bytes:  "c2xV4("★")
say "😀 4 bytes: "c2xV4("😀")
say ' '
say ' UTF8 4 bytes presentation blank separated, backward compatible'
say copies('-',100)
say "René:       "c2xV1("René")
say "Rene:       "c2xV1("Rene")
say "Adrian:     "c2xV1("Adrian")
say "Peter:      "c2xV1("Peter")
say "★ 3 bytes:  "c2xV1("★")
say "😀 4 bytes: "c2xV1("😀")
say ' '
say ' UTF8 4 bytes presentation, backward compatible'
say copies('-',100)
say "René:       "c2xV2("René")
say "Rene:       "c2xV2("Rene")
say "Adrian:     "c2xV2("Adrian")
say "Peter:      "c2xV2("Peter")
say "★ 3 bytes:  "c2xV2("★")
say "😀 4 bytes: "c2xV2("😀")
say ' NETREXX approach, just return first (UTF8) character'
say copies('-',100)
say "René:       "c2xV5("René")
say "Rene:       "c2xV5("Rene")
say "Adrian:     "c2xV5("Adrian")
say "Peter:      "c2xV5("Peter")
say "★ 3 bytes:  "c2xV5("★")
say "😀 4 bytes: "c2xV5("😀")
exit
c2xV1: procedure = .string
  arg from = .string
  stx=""
  len=0
  assembler strlen len,from
  if len=0 then return ""
  do  i=0 to len-1
      fz="UTFV1"
      assembler hexchar fz,from,i
      stx=stx||fz||' '
  end
return strip(stx)
c2xV2: procedure = .string
  arg from = .string
  stx=""
  len=0
  assembler strlen len,from
  if len=0 then return ""
  do  i=0 to len-1
      fz="UTFV1"
      assembler hexchar fz,from,i
      stx=stx||fz
  end
return stx
c2xV3: procedure = .string
  arg from = .string
  stx=""
  len=0
  assembler strlen len,from
  if len=0 then return ""
  do  i=0 to len-1
      fz="UTFV2"
      assembler hexchar fz,from,i
      stx=stx||fz
  end
return stx
c2xV4: procedure = .string
  arg from = .string
  stx=""
  len=0
  assembler strlen len,from
  if len=0 then return ""
  do  i=0 to len-1
      fz="UTFV2"
      assembler hexchar fz,from,i
      stx=stx||fz||' '
  end
return strip(stx)
c2xV5: procedure = .string
  arg from = .string
  len=0
  assembler strlen len,from
  if len=0 then return ""
  offset=0
  fz="UTFV2"
  assembler hexchar fz,from,offset
return fz
*/
