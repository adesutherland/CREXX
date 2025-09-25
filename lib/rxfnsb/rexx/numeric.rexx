/* Numeric Options Access Function
 *
 * Functions to get
 * - the number of significant digits for calculations - digits()
 * - the number of digits to ignore during numeric comparisons - fuzz()
 * - the preferred exponential notation format - form()
 * - the case sensitivity for special numeric literals - numcase()
 * - the arithmetic semantic rules - standard()
 */
options levelb

namespace rxfnsb expose digits form fuzz numcase standard

digits: procedure = .int
    numeric digits inherited
    d = .int
    assembler getnumdgts d
    return d

fuzz: procedure = .int
    numeric fuzz inherited
    f = .int
    assembler getnumfuz f
    return f

form: procedure = .string
    numeric form inherited
    f = .int
    assembler getnumfrm f
    if f = 1 then return "scientific"
    if f = 2 then return "engineering"
    return "unknown"

numcase: procedure = .string
    numeric case inherited
    c = .int
    assembler getnumcas c
    if c = 1 then return "lower"
    if c = 2 then return "upper"
    return "unknown"

standard: procedure = .string
    numeric standard inherited
    s = .int
    assembler getnumstd s
    if s = 1 then return "common"
    if s = 2 then return "classic"
    return "unknown"
