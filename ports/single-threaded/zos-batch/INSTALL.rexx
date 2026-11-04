/* REXX: cREXX TSO31 batch PoC installer. Run under IKJEFT1A. */
signal on syntax name failed
signal on halt name failed
address tso
created=''
attempted=''
stackactive=0; inputactive=0; oldprompt=''
keys='LOAD PROCLIB RXBIN SOURCE JCL'
cfg.=''
cfg.UNIT='SYSDA'
"EXECIO * DISKR CONFIG (STEM input. FINIS"
if rc<>0 then call die 'Cannot read CONFIG.'
do i=1 to input.0
  line=strip(input.i)
  if line='' | left(line,1)='*' then iterate
  if pos('=',line)=0 then call die 'CONFIG requires KEY=VALUE.'
  parse var line key '=' value
  key=translate(strip(key)); value=translate(strip(value))
  if wordpos(key,keys 'UNIT VOLUME')=0 then
    call die 'Unknown CONFIG key:' key
  if seen.key=1 then call die 'Duplicate CONFIG key:' key
  seen.key=1; cfg.key=value
end
do i=1 to words(keys)
  key=word(keys,i); target=cfg.key
  if valid(target)=0 then call die 'Invalid target dataset:' target
  do j=1 to i-1
    other=word(keys,j)
    if cfg.other=target then call die 'Targets must be distinct.'
  end
  status=sysdsn("'"target"'")
  if status<>'DATASET NOT FOUND' then
    call die 'Target not absent:' target status
end
if right(cfg.RXBIN,6)<>'.RXBIN' then
  call die 'This compiler requires the RXBIN suffix: root.RXBIN.'
root=left(cfg.RXBIN,length(cfg.RXBIN)-6)
if valid(root)=0 then call die 'Invalid compiler import root.'
if length(cfg.UNIT)>8 | cfg.UNIT='' then
  call die 'UNIT must contain 1-8 letters or digits.'
if verify(cfg.UNIT,'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789')<>0 then
  call die 'Invalid UNIT.'
if length(cfg.VOLUME)>6 then call die 'VOLUME exceeds 6 characters.'
if verify(cfg.VOLUME,'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789')<>0 then
  call die 'Invalid VOLUME.'
allocation='UNIT='cfg.UNIT','
restore='UNIT('cfg.UNIT')'
if cfg.VOLUME<>'' then do
  allocation=allocation'VOL=SER='cfg.VOLUME','
  restore=restore' VOLUME('cfg.VOLUME')'
end
if listdsi('DISTRIB FILE')<>0 then call die 'Cannot inspect DISTRIB.'
dist=sysdsname
if sysdsorg<>'PO' | sysrecfm<>'FB' | syslrecl<>80 then
  call die 'DISTRIB must be the received FB80 distribution PDS.'
/* Check every input before the first restore. */
do i=1 to words(keys)
  key=word(keys,i)
  if sysdsn("'"dist"("key")'")<>'OK' then
    call die 'Missing nested XMI:' key
end
/* Validate all configured text in memory before creating any target. */
members='CRXC CRXCG CRXCL CRXLGO IVP CG BAD C CL'
do m=1 to words(members)
  member=word(members,m)
  call readmember dist,member
  do n=1 to data.0
    text=render(strip(data.n,'T'))
    if length(text)>71 then call die 'JCL card too long:' member n
    save.m.n=text
  end
  count.m=data.0
end
/* NEWSTACK isolates RECEIVE replies from the caller's stack. */
do i=1 to words(keys)
  key=word(keys,i); target=cfg.key
  /* Recheck immediately: never request REPLACE or delete a target. */
  if sysdsn("'"target"'")<>'DATASET NOT FOUND' then
    call die 'Target became available or inaccessible:' target
  "ALLOC FI(CRXINPUT) DA('"dist"("key")') SHR"
  if rc<>0 then call die 'Cannot allocate nested XMI:' key
  inputactive=1
  oldprompt=prompt('ON')
  "NEWSTACK"
  if rc<>0 then call die 'NEWSTACK failed.'
  stackactive=1
  queue "DATASET('"target"')" restore
  attempted=target
  "RECEIVE INDDNAME(CRXINPUT)"
  receiveRC=rc; pending=queued()
  call cleanup
  if receiveRC<>0 | pending<>0 then
    call die 'RECEIVE failed:' key receiveRC
  created=created target
  say 'RESTORED' key target
  if sysdsn("'"target"'")<>'OK' then
    call die 'Restored target is unavailable:' target
end
/* Write only newly restored, private libraries. */
do m=1 to words(members)
  member=word(members,m)
  if m<=4 then target=cfg.PROCLIB
  else target=cfg.JCL
  data.0=count.m
  do n=1 to data.0
    data.n=save.m.n
  end
  "ALLOC FI(CRXEDIT) DA('"target"("member")') OLD"
  if rc<>0 then call die 'Cannot allocate configured member:' member
  "EXECIO" data.0 "DISKW CRXEDIT (STEM data. FINIS"
  writeRC=rc
  "FREE FI(CRXEDIT)"
  if writeRC<>0 then call die 'Cannot write configured member:' member
end
say 'cREXX BATCH INSTALL OK'
say 'Edit the site JOB cards in' cfg.JCL'.'
say 'Submit IVP: COMP/ASSEM/LINK/RUN must all return 0000.'
say 'RUN SYSPRINT must contain: cREXX BATCH IVP OK: new payload'
exit 0

readmember: procedure expose data. created attempted,
  stackactive inputactive oldprompt
  parse arg dataset,member
  address tso "ALLOC FI(CRXEDIT) DA('"dataset"("member")') SHR"
  if rc<>0 then call die 'Cannot read template:' member
  address tso "EXECIO * DISKR CRXEDIT (STEM data. FINIS"
  readRC=rc
  address tso "FREE FI(CRXEDIT)"
  if readRC<>0 then call die 'Template read failed:' member
  return

render: procedure expose cfg. keys root allocation created attempted,
  stackactive inputactive oldprompt
  parse arg text
  rendered=''
  do while pos('@',text)>0
    at=pos('@',text); last=pos('@',text,at+1)
    if last=0 then call die 'Unterminated template token.'
    key=substr(text,at+1,last-at-1)
    select
      when key='ROOT' then value=root
      when key='ALLOC' then value=allocation
      when wordpos(key,keys)>0 then value=cfg.key
      otherwise call die 'Unknown template token:' key
    end
    rendered=rendered||left(text,at-1)||value
    text=substr(text,last+1)
  end
  return rendered||text

valid: procedure
  parse arg name
  first='ABCDEFGHIJKLMNOPQRSTUVWXYZ@#$'
  rest=first'0123456789-'
  if name='' | length(name)>44 then return 0
  do while name<>''
    parse var name part '.' name
    if part='' | length(part)>8 then return 0
    if pos(left(part,1),first)=0 | verify(part,rest)<>0 then return 0
  end
  if right(arg(1),1)='.' then return 0
  return 1

failed:
  call die 'REXX failure at line' sigl
die:
  parse arg message
  call cleanup
  say 'cREXX INSTALL FAILED:' message
  say 'No existing target is replaced or deleted.'
  say 'Targets restored before failure:' created
  say 'Last restore target (possibly partial):' attempted
  say 'Review any partial installation before trying fresh names.'
  exit 12

cleanup: procedure expose stackactive inputactive oldprompt
  if stackactive=1 then do
    address tso 'DELSTACK'
    stackactive=0
  end
  if oldprompt<>'' then do
    ignored=prompt(oldprompt)
    oldprompt=''
  end
  if inputactive=1 then do
    address tso 'FREE FI(CRXINPUT)'
    inputactive=0
  end
  return
