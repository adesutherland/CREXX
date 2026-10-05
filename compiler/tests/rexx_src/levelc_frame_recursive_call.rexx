count = 0
call step
say count
exit
step:
count = count + 1
if count < 3 then call step
return
