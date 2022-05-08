/* COS: procedure  */
parse arg X, P 
if P = "" then P = 9; numeric digits P 
Pi = PICONST(); Signum = 1; X = ABS(X) 
Pim2 = Pi * 2; X = X // Pim2; Pid2 = Pi / 2 
if X > Pi 
  then do; X = X - Pi; Signum = -Signum; end 
if X > Pid2 
  then do; X = Pi - X; Signum = -Signum; end 
Term = 1; Xsup2 = X * X; Sum = 1; F = 1 
do J = 2 by 2 
  Term = -Term * Xsup2 / (J * (J - 1)) 
  NewSum = Sum + Term 
  if NewSum = Sum then return Signum * Sum 
  Sum = NewSum 
end