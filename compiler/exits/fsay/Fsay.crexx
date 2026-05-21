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
namespace rxcpexits expose fsayexit

import rxcp
import rxfnsb

fsayexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("fsay")
        call desc.add_import("rxfnsb", "descriptor", "")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        return .exitplan("READY")

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")

        if tokens.0 \= 2 then do
            call result.set_status("REJECT")
            return result
        end

        do i = 1 to tokens.0
            if i = 1 then iterate
            t_type = strip(tokens[i].get_type())
            if pos(t_type, "string_literal") > 0 then iterate
            call result.set_error(i, "Unsupported token type in FSAY: <" || t_type || "> text=<" || tokens[i].get_text() || ">")
            return result
        end

        do i = 2 to tokens.0
            if strip(tokens[i].get_type()) = "identifier" & tokens[i].get_value_type() = ".unknown" then do
                call result.set_status("PENDING")
                return result
            end
        end

        call result.set_status("REPLACE")
        _replacement='say 'fsayfmt(tokens[2].get_text())
##        call LineOut("c:\temp\CREXX\LOG.txt",_replacement)
        call result.add_replacement_line(_replacement)
        return result
