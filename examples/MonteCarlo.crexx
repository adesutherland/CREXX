/* pi.rexx
   Estimate pi using the Monte Carlo method.

   Random points are generated inside the unit square.
   The fraction that falls inside the quarter circle
   approximates pi / 4.
*/
options levelb
import rxfnsb

samples = 10000000
inside  = 0.0

x = .float
y = .float

do i = 1 to samples
    /* Generate random coordinates in the range 0.0 .. 1.0 */
    x = random(0, 1000) / 1000.0
    y = random(0, 1000) / 1000.0

    /* Count points inside the quarter circle */
    if x*x + y*y <= 1.0 then
        inside = inside + 1
end

say 'Inside ratio =' inside / samples
say 'Pi ≈' 4.0 * inside / samples