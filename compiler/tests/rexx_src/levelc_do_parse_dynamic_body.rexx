options levelc
delimiter=','
source='left,right'
do
  parse var source first (delimiter) second
end
say first || '|' || second
