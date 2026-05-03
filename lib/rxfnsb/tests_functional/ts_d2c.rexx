/* D2C */
options levelb numeric_classic
import rxfnsb

errors=0
say "Look for D2C OK"
if d2c(77) \= 'M' then do
  errors=errors+1
  say 'D2C failed in test 1'
end

if d2c(77,1) \= 'M' then do
  errors=errors+1
  say 'D2C failed in test 2'
end

if d2c(12,0) \= '' then do
  errors=errors+1
  say 'D2C failed in test 3'
end

if c2d(d2c(9)) \= 9 then do
  errors=errors+1
  say 'D2C failed in test 4'
end

if c2d(d2c(945)) \= 945 then do
  errors=errors+1
  say 'D2C failed in test 5'
end

if d2c(945) \= 'α' then do
  errors=errors+1
  say 'D2C failed in test 6'
end

if c2d(d2c(128293)) \= 128293 then do
  errors=errors+1
  say 'D2C failed in test 7'
end

if d2c(128293) \= '🔥' then do
  errors=errors+1
  say 'D2C failed in test 8'
end

if c2d('M') \= 77 then do
  errors=errors+1
  say 'C2D failed in test 9'
end

if c2d('α') \= 945 then do
  errors=errors+1
  say 'C2D failed in test 10'
end

if c2d('🔥') \= 128293 then do
  errors=errors+1
  say 'C2D failed in test 11'
end

return errors<>0
