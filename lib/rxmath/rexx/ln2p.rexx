/* LN2P: procedure  */
parse arg P 
if P <= 200 then return LN2() 
N = 1 / 3; Ln = N; Zetasup2 = 1 / 9 
do J = 1 
  N = N * Zetasup2; NewLn = Ln + N / (2 * J + 1) 
  if NewLn = Ln then return 2 * Ln 
  Ln = NewLn 
end
