/* rexx */
options levelb

namespace rxfnsb expose max

max: procedure = .float
  arg f1=.float,f2=.float,f3=-1e100,f4=-1e100,f5=-1e100,f6=-1e100,f7=-1e100,f8=-1e100,f9=-1e100,f10=-1e100,f11=-1e100,f12=-1e100,f13=-1e100,f14=-1e100,f15=-1e100,f16=-1e100
     fx=f1
     if f2>fx then fx=f2
     if f3>fx then fx=f3
     if f4>fx then fx=f4
     if f5>fx then fx=f5
     if f6>fx then fx=f6
     if f7>fx then fx=f7
     if f8>fx then fx=f8
     if f9>fx then fx=f9
     if f10>fx then fx=f10
     if f11>fx then fx=f11
     if f12>fx then fx=f12
     if f13>fx then fx=f13
     if f14>fx then fx=f14
     if f15>fx then fx=f15
     if f16>fx then fx=f16
  return fx


return 0

