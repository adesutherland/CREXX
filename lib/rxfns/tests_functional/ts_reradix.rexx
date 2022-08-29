/* rexx */
options levelb
import _rxsysb

/* These from TRL */
errors=0

if reradix('A',16,10) \= 10  then do
  errors=errors+1
  say 'RERADIX failed in test 1 reradix('A',10,16) ' reradix('A',10,16)
end

if reradix('81',16,10) \= 129  then do
  errors=errors+1
  say 'RERADIX failed in test 2 reradix('81',16,10) ' reradix('81',16,10)
end

if reradix('F81',16,10) \= 3969  then do
  errors=errors+1
  say 'RERADIX failed in test 3 reradix('F81',16,10)' reradix('F81',16,10)
end

if reradix('f81',16,10) \= 3969  then do
  errors=errors+1
  say 'RERADIX failed in test 4 reradix('f81',16,10)' reradix('f81',16,10) /* lower case */
end

if reradix('FF81',16,10) \= 65409  then do
  errors=errors+1
  say 'RERADIX failed in test 5 reradix('FF81',16,10)' reradix('FF81',16,10)
end

if reradix('FF81',16,10) \= '65409'  then do
  errors=errors+1
  say 'RERADIX failed in test 6 reradix('FF81',16,10)' reradix('FF81',16,10)
end

if reradix('F0',16,10) \= 240  then do
  errors=errors+1
  say 'RERADIX failed in test 7 reradix('F0',16,10)' reradix('F0',16,10)
end

if reradix('FF',16,10) \= 255  then do
  errors=errors+1
  say 'RERADIX failed in test 8 reradix('FF',16,10)' reradix('FF',16,10)
end

if reradix('80',16,10) \= 128  then do
  errors=errors+1
  say 'RERADIX failed in test 9 reradix('80',16,10)' reradix('80',16,10)
end

if reradix('baba',16,10) \= 47802  then do
  errors=errors+1
  say 'RERADIX failed in test 10 reradix('baba',16,10)' reradix('baba',16,10)
end

return errors<>0
