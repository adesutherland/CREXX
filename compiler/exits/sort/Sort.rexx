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
namespace rxcpexits expose sortexit

import rxcp
import rxfnsb

sortexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("sort")
        call desc.add_import("rxfnsb", "descriptor", "")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        return .exitplan("READY")

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")
        rule[2] = "identifier[]"
        rule[3] = "identifier/int_literal"
        rule[4] = "identifier/string_literal"
        rule[5] = "identifier/int_literal"

        if tokens.0 < 4 then do
            call result.set_status("REJECT")
            return result
        end

        do i = 2 to tokens.0
            if rule[i] = "" then iterate
            ti = tokens[i]
            t_type = strip(ti.get_type())
            if _type_allowed(t_type, rule[i]) then iterate
            call result.set_error(i, i || ". parameter <" || ti.get_text() || "> must be <" || rule[i] || ">, is <" || t_type || ">")
            return result
        end

        do i = 2 to tokens.0
            if rule[i] = "" then iterate
            ti = tokens[i]
            if strip(ti.get_type()) = "identifier" & ti.get_value_type() = ".unknown" then do
                call result.set_status("PENDING")
                return result
            end
        end

        do i = 2 to tokens.0
            if rule[i] = "" then iterate
            if pos("[", rule[i]) = 0 then iterate
            ti = tokens[i]
            if pos("[", strip(ti.get_value_type())) = 0 then do
                call result.set_error(i, i || ". parameter '" || ti.get_text() || "' must be a stem/array")
                return result
            end
        end

        if tokens.0 >= 5 then replacement = "__rc=arraysort({2},{3},{4},{5});"
        else if tokens.0 = 4 then replacement = "__rc=arraysort({2},{3},{4});"
        else replacement = "__rc=arraysort({2},{3});"

        call result.set_status("REPLACE")
        call result.add_replacement_line(replacement)
        return result

_type_allowed: procedure = .int
    arg t_type = .string, allowed = .string
    allowed = translate(allowed,, '/\\')
    do j = 1 to words(allowed)
        if t_type = word(allowed, j) then return 1
        if t_type || "[]" = word(allowed, j) then return 1
    end
    return 0
