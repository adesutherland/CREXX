/* Controlled variation: revised FDSqrt with consistently renamed locals.
 * The original door algorithm and SLAM attribution are in README.md. */
FDSqrt: procedure
  parse arg input
  if input<=0 then return 0
  estimate=input; if estimate<1 then estimate=1
  do forever
    candidate=(estimate+(input/estimate))/2
    if candidate>=estimate then leave
    estimate=candidate
  end
  return estimate
