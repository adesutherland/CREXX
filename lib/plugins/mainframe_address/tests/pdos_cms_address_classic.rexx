options levelc
/* Real commands and conditions in the bounded PDOS CMS personality. */
errors=0
failures=0
call on error name errored
call on failure name failed
address cms
if address()<>'CMS' then exit 2
do i=1 to 3
  'QUERY TIME'
  if rc<>0 | .rc<>0 | .rs<>0 then exit 3
end
'QUERY DISK'
if rc<>24 | .rc<>24 | .rs<>1 | errors<>1 then exit 4
say 'UNSUPPORTED CMS QUERY DISK AND ERROR PASS'
address cms with output stem captured.
'QUERY TIME'
if rc<>-3 | .rs<>-1 | captured.0<>0 | failures<>1 then exit 5
address cms with output normal
'QUERY TIME'
if rc<>0 then exit 6
address pdos 'ENVCHILD HELLO'
if rc<>7 | .rc<>7 | .rs<>1 | errors<>2 then exit 7
address pdos 'ENVFAULT'
if rc>=0 | .rc<>rc | .rs<>-1 | failures<>2 then exit 8
address pdos 'VERSION'
if rc<>0 then exit 9
address system 'QUERY TIME'
if rc<>0 then exit 10
address cms
'QUERY TIME'
if rc<>0 | .rc<>0 | .rs<>0 | address()<>'CMS' then exit 11
say 'NONZERO AND RECOVERY PASS PDOS'
say 'PASS: native ADDRESS Classic CMS'
exit 0
errored:
errors=errors+1
if condition('C')<>'ERROR' | .rs<>1 then exit 20
return
failed:
failures=failures+1
if condition('C')<>'FAILURE' | .rs<>-1 then exit 21
return
