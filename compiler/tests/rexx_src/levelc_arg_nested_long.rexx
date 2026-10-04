options levelc
call probe 'one two three four five six seven eight nine ten eleven twelve'
exit

probe:
procedure
if 1 then arg a b c d e f g h i j k l
do 2
  arg again
end
say a || '|' || f || '|' || l
say again
return
