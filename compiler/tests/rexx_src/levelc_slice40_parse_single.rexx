options levelc
source='aBc dEf'
parse var source plain
say 'plain' plain source
parse upper var source upper
say 'upper' upper source
parse value source || '!' with value
say 'value' value source
hits=0
parse value nextvalue() with once
say 'once' once hits
parse upper value 'mIx Ed' with self
say 'self' self
parse upper var self self
say 'same' self
if 1 then parse var source in_if
do
  parse upper var in_if in_do
end
say 'nested' in_do
say 'local' local('dEf')
exit
nextvalue: procedure expose hits
hits=hits+1
return 'new'
local: procedure
arg input
parse value 'mIx' with input
parse upper var input result
return result || '/' || input
