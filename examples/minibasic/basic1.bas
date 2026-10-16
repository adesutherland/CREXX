10 REM Calculate the sum from 1 to N
20 LET N = 10
30 LET I = 1
40 LET TOTAL = 0

50 IF I > N THEN 100
60 LET TOTAL = TOTAL + I
70 LET I = I + 1
80 GOTO 50

100 PRINT "SUM =", TOTAL
110 PRINT "Enter another number:"
120 INPUT VALUE
130 IF VALUE < 0 THEN 170
140 PRINT "The number is non-negative:", VALUE
150 GOTO 180

170 PRINT "The number is negative:", VALUE

180 END