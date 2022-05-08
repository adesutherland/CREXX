/* SQRT: procedure  */
parse arg N, P 
if P = "" then P = 9; numeric digits P 
parse value FORMAT(N,,,,0) with N "E" Exp 
if Exp = "" then Exp = 0 
if (Exp // 2) <> 0 then 
  if Exp > 0 
    then do; N = N * 10; Exp = Exp - 1; end 
    else do; N = N / 10; Exp = Exp + 1; end 
X = 0.5 * (N + 1) 
do forever 
  NewX = 0.5 * (X + N / X) 
  if X = NewX then return X * 10 ** (Exp % 2) 
  X = NewX 
end
