/* rexx test abs bif */
options levelb
say "Test MIN"
say min(100,99,1)
say min(47,42,129)
say min(1,9,10)

say "Test MAX"
say max(100,99,1)
say max(47,42,129)
say max(1,9,10)


return

min: procedure = .float
  arg f1=.float,f2=.float, f3=1e100,f4=1e100,f5=1e100,f6=1e100,f7=1e100,f8=1e100,f9=1e100,f10=1e100,f11=1e100,f12=1e100,f13=1e100,f14=1e100,f15=1e100,f16=1e100
max: procedure = .float
  arg f1=.float,f2=.float,f3=-1e100,f4=-1e100,f5=-1e100,f6=-1e100,f7=-1e100,f8=-1e100,f9=-1e100,f10=-1e100,f11=-1e100,f12=-1e100,f13=-1e100,f14=-1e100,f15=-1e100,f16=-1e100
