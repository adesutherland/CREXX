options levelb
namespace rxcpexits expose msayexit

import rxcp
import rxfnsb

msayexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("msay")
        call desc.add_import("rxfnsb", "descriptor", "")
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

        do i = 1 to tokens.0
            if i = 1 then iterate
            t_type = strip(tokens[i].get_type())
            if pos(t_type, "string_literal") > 0 then iterate
            if pos(t_type, "int_literal") > 0 then iterate
            if pos(t_type, "identifier") > 0 then iterate
            if pos(t_type, "comma") > 0 then iterate
            if pos(t_type, "bracket") > 0 then iterate
            call result.set_error(i, "Unsupported token type in MSAY: <" || t_type || "> text=<" || tokens[i].get_text() || ">")
            return result
        end

        do i = 2 to tokens.0
            if strip(tokens[i].get_type()) = "identifier" & tokens[i].get_value_type() = ".unknown" then do
               if substr(tokens[i].get_value_type(),1,1)='.' then nop   /* assume that it's part of a stem, jump w.o. safety net */
               else do
                 call result.set_status("PENDING")
                 return result
               end
            end
        end
        parms=''
        do i = 2 to tokens.0
           parms=parms||tokens[i].get_text()
        end
        call result.set_status("REPLACE")
        _replacement='say fmtmask('parms')'
##         call LineOut("c:\temp\CREXX\LOG.txt",987 _replacement)
        call result.add_replacement_line(_replacement)
        return result
