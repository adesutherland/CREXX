/* Level B Function Test */
options levelb

say "starting"
start_time = 0
end_time = 0
a = 0
assembler time start_time
do i = 1 to 100000000
  a = a + i
end
assembler time end_time
say "Time taken is" end_time-start_time "seconds"
