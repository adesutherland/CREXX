/*
 * cREXX License (MIT)
 *
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, René Jansen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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