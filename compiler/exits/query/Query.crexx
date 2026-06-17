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
namespace rxcpexits expose queryexit

import rxcp
import rxfnsb

queryexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("query")
        call desc.add_additional_keyword("crexx")
        call desc.add_additional_keyword("os")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        return .exitplan("READY")

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")

        if tokens.0 < 2 then do
            call result.set_error(1, "Missing query argument (crexx or os)")
            return result
        end

        arg1 = lower(tokens[2].get_text())

        if arg1 = "crexx" then do
            call result.set_status("REPLACE")
            call result.add_replacement_line("do; _q_info = ''; assembler rxvers _q_info; say 'cREXX Version ' || word(_q_info, 3); end;")
            return result
        end

        if arg1 = "os" then do
            call result.set_status("REPLACE")
            call result.add_replacement_line("do; _q_info = ''; assembler rxvers _q_info; say 'OS: ' || word(_q_info, 1); end;")
            return result
        end

        call result.set_error(2, "Invalid query argument: " || tokens[2].get_text())
        return result
