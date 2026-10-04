options levelc
source='a,b'
delimiter=','
parse var source first (delimiter) second
say first || '|' || second
