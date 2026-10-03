options levelc
flag = 0
if 1 then say 'true'
else say 'bad'
if 0 then say 'bad'
else say 'false'
if 1 then if 0 then say 'bad'
else say 'nested'
say 'after'
if mark() then say 'once'
say flag
if 0 then say mark()
say flag
exit
mark:
procedure expose flag
flag = flag + 1
if flag = 1 then return 1
return 0
