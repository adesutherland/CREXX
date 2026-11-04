/* REXX: native XMIT builder, run only by the Mainframe Operator.
 * Input DDs: INLOAD INRXBIN INPROC INSOURCE INJCL (owned PDSes).
 * Prefix names DIST (FB80 PDS with INSTALL and nine text templates)
 * and OUTER (empty preallocated FB80 PS). Outputs must be fresh. */
parse upper arg producer addressee
if producer='' | length(producer)>36 then exit 2
if verify(producer,'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.@#$-')<>0 then
  exit 2
parse var addressee node '.' userid
if node='' | userid='' | length(node)>8 | length(userid)>8 then exit 2
if verify(node||userid,'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$')<>0 then
  exit 2
address tso
names='LOAD RXBIN PROCLIB SOURCE JCL'
ddnames='INLOAD INRXBIN INPROC INSOURCE INJCL'
dist=producer'.DIST'; outer=producer'.OUTER'
if sysdsn("'"dist"'")<>'OK' then exit 12
if sysdsn("'"outer"'")<>'OK' then exit 12
/* Preflight every nested input/output before creating any member. */
do i=1 to words(names)
  key=word(names,i); ddname=word(ddnames,i)
  output=dist'('key')'
  if sysdsn("'"output"'")<>'MEMBER NOT FOUND' then exit 12
  if listdsi(ddname 'FILE')<>0 then exit 12
  if sysdsorg<>'PO' then exit 12
  inputs.i=sysdsname
  say 'INPUT DCB' key sysdsorg sysrecfm syslrecl sysblksize
end
do i=1 to words(names)
  key=word(names,i); input=inputs.i; output=dist'('key')'
  "ALLOC FI(RNOUT) DA('"output"') SHR"
  if rc<>0 then exit 12
  "TRANSMIT" addressee "DATASET('"input"')",
    'OUTDDNAME(RNOUT) NONOTIFY NOLOG'
  transmitRC=rc
  'FREE FI(RNOUT)'
  if transmitRC<>0 then exit 12
  say 'NESTED XMI OK' key
end
"ALLOC FI(RNOUT) DA('"outer"') OLD"
if rc<>0 then exit 12
"TRANSMIT" addressee "DATASET('"dist"')",
  'OUTDDNAME(RNOUT) NONOTIFY NOLOG'
transmitRC=rc
'FREE FI(RNOUT)'
if transmitRC<>0 then exit 12
say 'CREXX OUTER XMI OK'
exit 0
