options levelb

errors=0
/*- - C O N C A T E N A T I O N - -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --*/
if 5  1 ='5 1' then ok=ok ! '<blank>'
else do
  errors=errors+1
  say '*** Bad *** <blank> operator'
end
if 6||2 ='62'  then ok=ok ! '||'
else do
  errors=errors+1
  say '*** Bad *** || operator'
end
if 'X'123.4'y' ='X123.4y' then ok=ok ! '<implicit Concat>'
else do
  errors=errors+1
  say '*** Bad *** <implicit concat> operator'
end

if 'X'123.4/10'y' ='X12.34y' then ok=ok ! '<compound Concat>'
else do
  errors=errors+1
  say '*** Bad *** <compound Concat> operator'
end
/*- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -*/

return errors<>0