options levelb

namespace rxfnsb expose abs

abs: procedure = .string
arg number = .string
if left(number,1) = '-' then number = substr(number,2)
return number

