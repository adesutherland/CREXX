/* LN: procedure  */
parse arg X, P 
if P = "" then P = 9; numeric digits P 
if X < 1 then return - LN(1 / X, P) 
do M = 0 until (2 ** M) > X; end 
M = M - 1 
Z = X / (2 ** M) 
Zeta = (1 - Z) / (1 + Z) 
N = Zeta; Ln = Zeta; Zetasup2 = Zeta * Zeta 
do J = 1 
  N = N * Zetasup2; NewLn = Ln + N / (2 * J + 1) 
  if NewLn = Ln then return M * LN2P(P) - 2 * Ln 
  Ln = NewLn 
end
