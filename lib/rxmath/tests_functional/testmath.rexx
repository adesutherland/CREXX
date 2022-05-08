/* test CREXX math functions */
call charout, left('fact(108001) = 8.14285997E+496713',40)
if fact(108001) == 8.14285997E+496713 then say ' ok'; else say 'fail'
call charout, left('PI(5)        = 3.1418',40)
if PI(5) == 3.1418 then say ' ok'; else say 'fail'
call charout, left('SQRT(2)      = 1.41421356',40)
if SQRT(2) == 1.41421356 then say ' ok'; else say 'fail'
call charout, left('SQRT(2,18)   = 1.41421356237309505',40)
if SQRT(2,18) == 1.41421356237309505 then say ' ok'; else say 'fail'
