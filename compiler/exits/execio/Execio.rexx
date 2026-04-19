options levelb
namespace rxcpexits expose execioexit

import rxcp
import rxfnsb

execioexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("execio")
        call desc.add_import("rxfnsb", "descriptor", "")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        compile_plan = .exitplan
        compile_plan = .exitplan("READY")

        do i = 2 to tokens.0
            if translate(tokens[i].get_text()) = "STEM" then do
                p = i + 1
                if p <= tokens.0 & strip(tokens[p].get_type()) = "identifier" then do
                    call compile_plan.add_binding("var", tokens[p].get_text(), "", ".string", 1, "execio_stem", "")
                    return compile_plan
                end
            end
        end
        return compile_plan

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")

        if tokens.0 < 4 then do
            call result.set_status("REJECT")
            return result
        end

        allowed = "identifier int_literal string_literal operator bracket comma other"
        do i = 1 to tokens.0
            ti = tokens[i]
            t_type = strip(ti.get_type())
            if i = 1 then iterate
            if pos(t_type, allowed) > 0 then iterate
            return error_result(i, "Unsupported token type in EXECIO: <" || t_type || "> text=<" || ti.get_text() || ">")
        end

        do i = 2 to tokens.0
            ti = tokens[i]
            if strip(ti.get_type()) = "identifier" & ti.get_value_type() = ".unknown" then do
                is_stem = 0
                if i > 2 then do
                    if translate(tokens[i-1].get_text()) = "STEM" then is_stem = 1
                end
                if \is_stem then do
                    call result.set_status("PENDING")
                    return result
                end
            end
        end

        ops = "DISKR DISKW DISKA READ WRITE APPEND DISKRU DISKSU"
        opPos = 0
        parenDepth = 0
        do i = 2 to tokens.0
            txt = tokens[i].get_text()
            if txt = "(" then parenDepth = parenDepth + 1
            else if txt = ")" then parenDepth = parenDepth - 1
            if parenDepth \= 0 then iterate
            if pos(translate(txt), ops) > 0 then do
                opPos = i
                leave
            end
        end
        if opPos = 0 then return error_result(2, "EXECIO: missing operation (DISKR/DISKW/...)")
        if opPos + 1 > tokens.0 then return error_result(opPos, "EXECIO: missing ddname/filename after operation")

        countExpr = ""
        do i = 2 to opPos - 1
            countExpr = countExpr || tokens[i].get_text()
        end
        if strip(countExpr) = "" then return error_result(2, "EXECIO: missing record count")

        if strip(countExpr) = "*" then countEmit = "'" || countExpr || "'"
        else countEmit = countExpr

        opEmit = upper(strip(tokens[opPos].get_text()))
        fnameTok = tokens[opPos + 1]
        if strip(fnameTok.get_type()) = "string_literal" then fnameEmit = fnameTok.get_text()
        else fnameEmit = "'" || fnameTok.get_text() || "'"

        stemPresent = 0
        stemEmit = ""
        p = opPos + 2
        if p <= tokens.0 & tokens[p].get_text() = "(" then do
            p = p + 1
            do while p <= tokens.0
                txt = tokens[p].get_text()
                up = translate(txt)
                if txt = ")" then do
                    p = p + 1
                    leave
                end
                if up = "STEM" then do
                    p = p + 1
                    if p > tokens.0 then return error_result(p - 1, "EXECIO: STEM requires a value")
                    vtok = tokens[p]
                    if strip(vtok.get_type()) \= "identifier" then return error_result(p, "EXECIO: STEM value " || vtok.get_text() || " mandatory stem (no ending dot)")
                    stemPresent = 1
                    stemEmit = vtok.get_text()
                    p = p + 1
                    iterate
                end
                if up = "FINIS" | up = "FIFO" | up = "LIFO" | up = "OPEN" then do
                    p = p + 1
                    iterate
                end
                return error_result(p, "EXECIO: unknown option '" || txt || "'")
            end
        end

        replacement = "__rc=_execio(" || countEmit || ",'" || opEmit || "'," || fnameEmit
        if stemPresent then replacement = replacement || ", " || stemEmit
        replacement = replacement || ")"

        call result.set_status("REPLACE")
        call result.add_replacement_line(replacement)
        return result

error_result: procedure = .exitresult
    arg token_index = .int, message = .string
    result = .exitresult("ERROR")
    call result.set_error(token_index, message)
    return result
