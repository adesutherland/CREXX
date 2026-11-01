10 REM Existing scalar string forms, including doubled quotes and empty values
20 PRINT "direct ""quote"""
30 LET TEXT = "assigned ""quote"""
40 LET COPY = TEXT
50 PRINT TEXT
60 PRINT COPY
70 PRINT (TEXT)
80 PRINT "left", "", "right"
90 LET EMPTY = ""
100 PRINT "left", EMPTY, "right"
110 PRINT ""
120 END
