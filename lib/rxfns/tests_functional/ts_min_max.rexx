/* rexx test abs bif */
options levelb
say "Test MIN"
errors=0

if min(100,99,1) \= 1 then do
  errors=errors+1
  say 'MIN failed in test 1'
end

if min(47,42,129) \= 42 then do
  errors=errors+1
  say 'MIN failed in test 2'
end

if min(1,9,10) \= 1 then do
  errors=errors+1
  say 'MIN failed in test 3'
end

say "Test MAX"
if  max(100,99,1) \= 100 then do
  errors=errors+1
  say 'MAX failed in test 1'
end

if  max(47,42,129) \= 129 then do
  errors=errors+1
  say 'MAX failed in test 2'
end

if max(1,9,10) \= 10 then do
  errors=errors+1
  say 'MAX failed in test 3'
end

return errors<>0

min: procedure = .float
  arg f1=.float,f2=.float, f3=1e100,f4=1e100,f5=1e100,f6=1e100,f7=1e100,f8=1e100,f9=1e100,f10=1e100,f11=1e100,f12=1e100,f13=1e100,f14=1e100,f15=1e100,f16=1e100
max: procedure = .float
  arg f1=.float,f2=.float,f3=-1e100,f4=-1e100,f5=-1e100,f6=-1e100,f7=-1e100,f8=-1e100,f9=-1e100,f10=-1e100,f11=-1e100,f12=-1e100,f13=-1e100,f14=-1e100,f15=-1e100,f16=-1e100
