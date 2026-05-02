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
namespace rxcpexits expose dumpexit

import rxcp
import rxfnsb

dumpexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("dump")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        return .exitplan("READY")

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")

        if tokens.0 < 1 then do
            call result.set_status("REJECT")
            return result
        end

        do i = 2 to tokens.0
            ti = tokens[i]
            if ti.get_type() \= "identifier" then do
                call result.set_error(i, "Not an identifier")
                return result
            end
        end

        do i = 2 to tokens.0
            ti = tokens[i]
            if ti.get_value_type() = ".unknown" then do
                call result.set_status("PENDING")
                return result
            end
        end

        replacement = ""
        do i = 2 to tokens.0
            ti = tokens[i]
            t_text = ti.get_text()
            t_type = ti.get_value_type()
            if pos("[", t_type) > 0 then do
                replacement = replacement || "do __rxcpx_dump_i = 1 to {" || i || "}[0]; say '" || t_text || " (" || t_type || ")[' || __rxcpx_dump_i || '] = ' || {" || i || "}[__rxcpx_dump_i]; end;"
            end
            else do
                replacement = replacement || "say '" || t_text || " (" || t_type || ") = ' || {" || i || "};"
            end
        end

        call result.set_status("REPLACE")
        call result.add_replacement_line(replacement)
        return result
