/* PI: procedure  */
parse arg P 
if P = "" then P = 9; numeric digits P 
X = SQRT(2, P); Pi = 2 + X 
Y = SQRT(X, P); X = Y 
do forever 
  X = 0.5 * (X + 1 / X) 
  NewPi = Pi * (X + 1) / (Y + 1) 
  if Pi = NewPi then return Pi 
  Pi = NewPi 
  X = SQRT(X, P) 
  Y = (Y * X + 1 / X) / (Y + 1) 
end -- do forever
