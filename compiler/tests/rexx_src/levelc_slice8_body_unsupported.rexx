options levelc
delimiter=','
do
  parse var source first (delimiter) second
end
