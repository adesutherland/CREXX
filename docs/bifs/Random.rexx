 /************************************************************************
 *.  R A N D O M   9.8.3
 ************************************************************************/
 call CheckArgs  'oWHOLE>=0 oWHOLE>=0 oWHOLE>=0'
 
 if !Bif_Arg.0 = 1 then do
   Minimum = 0
   Maximum = !Bif_Arg.1
   if Maximum>100000 then
     call Raise 40.31, Maximum
 end
else do
  if !Bif_ArgExists.1 then Minimum = !Bif_Arg.1
  else Minimum = 0
  if !Bif_ArgExists.2 then Maximum = !Bif_Arg.2
  else Maximum = 999
end

if Maximum-Minimum>100000 then
  call Raise 40.32, Minimum, Maximum

if Maximum-Minimum<0 then
  call Raise 40.33, Minimum, Maximum

if !Bif_ArgExists.3 then call Config_Random_Seed !Bif_Arg.3
call Config_Random_Next Minimum, Maximum
return !Outcome
