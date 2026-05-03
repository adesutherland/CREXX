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

options levelb
namespace rxcpexits expose addexit

import rxcp
import rxfnsb

addexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("add")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        return .exitplan("READY")

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")

        if tokens.0 < 3 then do
            call result.set_status("REJECT")
            return result
        end

        allowed = "identifier int_literal string_literal operator bracket comma other"
        do i = 1 to tokens.0
            ti = tokens[i]
            t_type = strip(ti.get_type())
            if i = 1 then iterate
            if pos(t_type, allowed) > 0 then iterate
            return error_result(i, "Unsupported token type in ADD: <" || t_type || "> text=<" || ti.get_text() || ">")
        end

        do i = 2 to tokens.0
            ti = tokens[i]
            if strip(ti.get_type()) = "identifier" & ti.get_value_type() = ".unknown" then do
                call result.set_status("PENDING")
                return result
            end
        end

        if check_array(tokens[2].get_value_type()) = 0 then do
            return error_result(2, "2. parameter <" || tokens[2].get_text() || "> must be a stem/array")
        end

        count_expr = ""
        do i = 3 to tokens[0]
            count_expr = count_expr || tokens[i].get_text()
        end

        call result.set_status("REPLACE")
        call result.add_replacement_line("{2}[{2}[0]+1]=" || count_expr)
        return result

check_array: procedure = .int
    arg array = .string
    t_val = strip(array)
    if pos("[", t_val) = 0 then return 0
    return 1

error_result: procedure = .exitresult
    arg token_index = .int, message = .string
    result = .exitresult("ERROR")
    call result.set_error(token_index, message)
    return result
