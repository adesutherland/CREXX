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
