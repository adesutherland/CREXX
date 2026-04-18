options levelb
namespace rxcpexits expose queryexit

import rxcp
import rxfnsb

queryexit: class
    _node_id = .int with register.1

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
