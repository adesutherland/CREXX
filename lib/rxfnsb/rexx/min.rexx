/* rexx */
options levelb

namespace rxfnsb expose min

min: procedure = .float
    arg m = .float, ... = .float
    do i = 1 to arg.0
        if arg.i < m then m = arg.i
    end
    return m
