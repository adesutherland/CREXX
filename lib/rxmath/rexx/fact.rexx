/* FACT: procedure  */
options levelb

fact: procedure = .int 
arg N = .int
F = 1 
do J = 2 to N
  F = F * J
end 
return F
  