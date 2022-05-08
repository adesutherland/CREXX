/* SIN: procedure  */
parse arg X, P 
if P = "" then P = 9; numeric digits P 
Pi = PICONST(); Signum = 1 
if X < 0 then do; Signum = -1; X = -X; end 
Pim2 = Pi * 2; X = X // Pim2; Pid2 = Pi / 2 
if X > Pi 
  then do; X = X - Pi; Signum = -Signum; end 
if X > Pid2 then X = Pi - X 
Term = X; Xsup2 = X * X; Sum = X; F = 1 
do J = 3 by 2 
  Term = -Term * Xsup2 / (J * (J - 1)) 
  NewSum = Sum + Term 
  if NewSum = Sum then return Signum * Sum 
  Sum = NewSum 
end
