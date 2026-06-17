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
namespace rxcpexits expose dummyexit

import rxcp

dummyexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("dummy")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]

        plan = .exitplan
        plan = .exitplan("READY")
        helper = .helperplan
        helper = .helperplan("dummyexit_helper_v2", "file_tail", "dummyexit_helper_v2", "")

        call helper.add_line("dummyexit_helper_v2: procedure")
        call helper.add_line("  say 'helper invoked'")
        call helper.add_line("return")
        call plan.add_helper(helper)

        if tokens.0 >= 2 & tokens[2].get_type() = "identifier" then do
            call plan.add_binding("var", tokens[2].get_text(), "", ".unknown", 0, "dummy_shadow", "")
        end

        return plan

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")

        if tokens.0 \= 2 then do
            call result.set_error(1, "INVALID_ARGUMENTS")
            return result
        end

        ti = tokens[2]
        if ti.get_type() \= "identifier" then do
            call result.set_error(2, "Not an identifier")
            return result
        end

        if ti.get_value_type() = ".unknown" then do
            call result.set_status("PENDING")
            return result
        end

        name = ti.get_text()
        call result.set_status("REPLACE")
        call result.add_replacement_line("say '" || name || " (" || ti.get_value_type() || ") = ' || {2}")
        call result.add_replacement_line("call dummyexit_helper_v2")
        call result.add_replacement_line(name || " = .int")
        call result.add_replacement_line(name || " = 1")
        call result.add_replacement_line("say 'in-exit ' || " || name)
        call result.add_replacement_line("dummy_var = 'hoisted_ok'")
        return result
