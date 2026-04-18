options levelb
namespace rxcpexits expose fsayexit

import rxcp
import rxfnsb

fsayexit: class
    _node_id = .int with register.1

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
        call result.add_replacement_line("say " || "fsayfmt(" || tokens[2].get_text() || ")")
        return result
