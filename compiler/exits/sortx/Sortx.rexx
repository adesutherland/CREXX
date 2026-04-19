options levelb
namespace rxcpexits expose sortexit2

import rxcp
import rxfnsb

sortexit2: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("sortx")
        call desc.add_import("rxfnsb", "descriptor", "")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        return .exitplan("READY")

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")

        if tokens[0] < 4 then do
            call result.set_status("REJECT")
            return result
        end

        do i = 2 to tokens[0]
            ti = tokens[i]
            t_type = strip(ti.get_type())
            if t_type = "identifier" | t_type = "int_literal" | t_type = "string_literal" | ,
               t_type = "operator" | t_type = "bracket" | t_type = "comma" | t_type = "other" then iterate
            call result.set_error(i, "Unsupported token type in sortx: <" || t_type || "> text=<" || ti.get_text() || ">")
            return result
        end

        do i = 2 to tokens[0]
            ti = tokens[i]
            if strip(ti.get_type()) = "identifier" & ti.get_value_type() = ".unknown" then do
                call result.set_status("PENDING")
                return result
            end
        end

        ranges = .string[]
        split_indx = 2
        parenDepth = 0
        do i = 2 to tokens[0]
            txt = tokens[i].get_text()
            if txt = "(" then parenDepth = parenDepth + 1
            else if txt = ")" then do
                parenDepth = parenDepth - 1
                if parenDepth < 0 then return error_result(i, "Unbalanced ')'")
            end
            if txt = "," & parenDepth = 0 then do
                if i = split_indx then return error_result(i, "Empty parameter before comma")
                ranges[ranges[0] + 1] = split_indx || " " || i - 1
                split_indx = i + 1
            end
        end

        if parenDepth \= 0 then return error_result(1, "Unbalanced parentheses in parameter list")
        if split_indx > tokens[0] then return error_result(1, "Empty final parameter")
        ranges[ranges[0] + 1] = split_indx || " " || tokens[0]

        args = .string[]
        do j = 1 to ranges[0]
            split_start = word(ranges[j], 1)
            split_end = word(ranges[j], 2)
            joined_parm = ""
            do i = split_start to split_end
                joined_parm = joined_parm || tokens[i].get_text()
            end
            if strip(joined_parm) = "" then return error_result(split_start, "Empty expression")
            args[args[0] + 1] = joined_parm
        end

        if pos("[", strip(tokens[2].get_value_type())) = 0 then do
            return error_result(2, "2. parameter <" || tokens[2].get_text() || "> must be a stem/array")
        end

        parms = args[1]
        do j = 2 to args[0]
            parms = parms || "," || args[j]
        end

        call result.set_status("REPLACE")
        call result.add_replacement_line("__rc=arraysort(" || parms || ")")
        return result

error_result: procedure = .exitresult
    arg token_index = .int, message = .string
    result = .exitresult("ERROR")
    call result.set_error(token_index, message)
    return result
