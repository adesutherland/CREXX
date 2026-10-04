options levelc
delimiter=','
source='left,right'
if 1 then parse var source first (delimiter) second
say first || '|' || second
