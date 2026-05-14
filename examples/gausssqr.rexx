/* sqrt_newton.rexx
   Newton's method computes the square root of a number by
   repeatedly improving an estimate until it converges.
*/
options levelb
import rxfnsb

sqrtof   = 3.14
estimate = 1.0              /* Initial guess */

do 1000
   /* Newton iteration: next = (estimate + x / estimate) / 2 */
   estimate = (estimate + sqrtof / estimate) / 2
end

say 'sqrt('sqrtof') =' estimate