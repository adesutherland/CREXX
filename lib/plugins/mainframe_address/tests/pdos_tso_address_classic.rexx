/* PDOS TSO personality fixture: use the qualified VERSION command.
   Original native CMS/TSO campaign source remains unchanged. */
options levelc
parse upper arg target capture_mode pdos qualify fault_dataset
/* Native argv may supply each word separately. Also accept the usual single
   Classic argument string, used by callers with a quoted command tail. */
if capture_mode = '' then capture_mode = translate(arg(2))
if pdos = '' then pdos = translate(arg(3))
if qualify = '' then qualify = translate(arg(4))
if fault_dataset = '' then fault_dataset = translate(arg(5))
if target <> 'CMS' & target <> 'TSO' then exit 2
command = 'TIME'
if target = 'CMS' then command = 'QUERY DISK'
address value target
if address() <> target then exit 3
do i = 1 to 3
  command
  if rc <> 0 | .rc <> 0 | .rs <> 0 then exit 4
end
errors = 0
failures = 0
call on error name errored
call on failure name failed
'BAD@CMD'
if rc = 0 | errors + failures <> 1 then exit 5
command
if rc <> 0 | .rc <> 0 | .rs <> 0 then exit 6
address value target with output stem captured.
command
if capture_mode = 'CAPTURE' then do
  if rc <> 0 | captured.0 = 0 then exit 7
  say 'CAPTURE:' captured.0
  do i = 1 to captured.0
    say 'RECORD:' captured.i
  end
end
else if rc <> -3 | .rs <> -1 | captured.0 <> 0 then exit 8
address value target with output normal
command
if rc <> 0 then exit 9
address system command
if rc <> 0 then exit 10
address value target
if address() <> target then exit 11
if pdos = 'PDOS' then do
  address pdos 'VERSION'
  if rc <> 0 then exit 12
end
if qualify = 'QUALIFY' then do
  route = target
  positive = 'STATE MFN404 DATA A'
  if target = 'TSO' then positive = "LISTDS 'MFADDR.NOEXIST.DATA'"
  if pdos = 'PDOS' then do
    route = 'PDOS'
    positive = 'ENVCHILD HELLO'
  end
  before = errors
  address value route
  positive
  if rc <= 0 | .rc <> rc | .rs <> 1 | errors <> before + 1 then exit 13
  if route = 'PDOS' then do
    if rc <> 7 then exit 14
    before = failures
    'ENVFAULT'
    if rc >= 0 | .rc <> rc | .rs <> -1 | failures <> before + 1 then exit 15
    'VERSION'
    if rc <> 0 then exit 16
  end
  else if target = 'TSO' & fault_dataset <> '' & fault_dataset <> '-' then do
    before = failures
    fault = "CALL '" || fault_dataset || "(MFFLT31)'"
    fault
    if rc >= 0 | .rc <> rc | .rs <> -1 | failures <> before + 1 then exit 18
    say 'NATIVE TSO FAULT RECOVERY PASS'
  end
  address value target
  command
  if rc <> 0 | .rc <> 0 | .rs <> 0 then exit 17
  say 'NONZERO AND RECOVERY PASS' route
end
say 'PASS: native ADDRESS Classic' target
exit 0
errored:
errors = errors + 1
if condition('C') <> 'ERROR' | .rs <> 1 then exit 20
return
failed:
failures = failures + 1
if condition('C') <> 'FAILURE' | .rs <> -1 then exit 21
return
