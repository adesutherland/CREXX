options levelb
namespace rxcpexits expose addressexit

import rxcp
import rxfnsb

addressexit: class
    _node_id = .int with register.1

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("ADDRESS")
        call desc.add_flag("certified")
        call desc.add_flag("reserved_keyword")
        call desc.add_flag("implicit_command")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        return .exitplan("READY")

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("EMPTY")

        explicit = .int
        env_expr = .string
        clause_start = .int
        command_start = .int
        command_end = .int
        input_start = .int
        input_end = .int
        output_start = .int
        output_end = .int
        error_start = .int
        error_end = .int
        expose_start = .int
        expose_end = .int
        section = .string
        depth = .int

        if tokens.0 < 1 then return error_result(1, "ADDRESS missing token stream")

        explicit = 0
        if upper(strip(tokens[1].get_text())) = "ADDRESS" then explicit = 1

        if explicit = 1 then do
            if tokens.0 < 3 then return error_result(1, "ADDRESS requires an environment and command")
            env_expr = emitEnvironment(tokens[2])
            clause_start = 3
        end
        else do
            env_expr = "'SYSTEM'"
            clause_start = 1
        end

        command_start = clause_start
        command_end = tokens.0
        input_start = 0
        input_end = -1
        output_start = 0
        output_end = -1
        error_start = 0
        error_end = -1
        expose_start = 0
        expose_end = -1
        section = "COMMAND"
        depth = 0

        do i = clause_start to tokens.0
            ti = tokens[i]
            type = strip(ti.get_type())
            text = strip(ti.get_text())
            utext = upper(text)

            if depth = 0 & isClauseKeyword(utext) then do
                if section = "COMMAND" then command_end = i - 1
                else if section = "INPUT" then input_end = i - 1
                else if section = "OUTPUT" then output_end = i - 1
                else if section = "ERROR" then error_end = i - 1
                else if section = "EXPOSE" then expose_end = i - 1
                if utext = "INPUT" then do
                    if input_start > 0 then return error_result(i, "ADDRESS INPUT clause repeated")
                    input_start = i + 1
                end
                else if utext = "OUTPUT" then do
                    if output_start > 0 then return error_result(i, "ADDRESS OUTPUT clause repeated")
                    output_start = i + 1
                end
                else if utext = "ERROR" then do
                    if error_start > 0 then return error_result(i, "ADDRESS ERROR clause repeated")
                    error_start = i + 1
                end
                else if utext = "EXPOSE" then do
                    if expose_start > 0 then return error_result(i, "ADDRESS EXPOSE clause repeated")
                    expose_start = i + 1
                end
                section = utext
                if i = tokens.0 then return error_result(i, "ADDRESS clause requires an operand")
                iterate
            end

            if type = "bracket" then do
                if text = "(" | text = "[" then depth = depth + 1
                else if text = ")" | text = "]" then depth = depth - 1
            end
        end

        if section = "COMMAND" then command_end = tokens.0
        else if section = "INPUT" then input_end = tokens.0
        else if section = "OUTPUT" then output_end = tokens.0
        else if section = "ERROR" then error_end = tokens.0
        else if section = "EXPOSE" then expose_end = tokens.0

        if command_end < command_start then return error_result(clause_start, "ADDRESS requires a command expression")
        if input_start > 0 & input_end < input_start then return error_result(input_start - 1, "ADDRESS INPUT clause requires an operand")
        if output_start > 0 & output_end < output_start then return error_result(output_start - 1, "ADDRESS OUTPUT clause requires an operand")
        if error_start > 0 & error_end < error_start then return error_result(error_start - 1, "ADDRESS ERROR clause requires an operand")
        if expose_start > 0 & expose_end < expose_start then return error_result(expose_start - 1, "ADDRESS EXPOSE clause requires an operand")
        if redirectNeedsResolution(tokens, input_start, input_end) then return pending_result()
        if redirectNeedsResolution(tokens, output_start, output_end) then return pending_result()
        if redirectNeedsResolution(tokens, error_start, error_end) then return pending_result()

        replacement = "rc=_address(" || env_expr
        replacement = replacement || "," || emitTokenRange(tokens, command_start, command_end)
        replacement = replacement || "," || buildRedirect(tokens, input_start, input_end, "IN")
        replacement = replacement || "," || buildRedirect(tokens, output_start, output_end, "OUT")
        replacement = replacement || "," || buildRedirect(tokens, error_start, error_end, "OUT")
        replacement = replacement || buildExposeArgs(tokens, expose_start, expose_end)
        replacement = replacement || ")"

        call result.set_status("REPLACE")
        call result.add_replacement_line(replacement)
        return result

pending_result: procedure = .exitresult
    result = .exitresult("PENDING")
    return result

error_result: procedure = .exitresult
    arg token_index = .int, message = .string
    result = .exitresult("ERROR")
    call result.set_error(token_index, message)
    return result

isClauseKeyword: procedure = .int
    arg text = .string
    if text = "INPUT" | text = "OUTPUT" | text = "ERROR" | text = "EXPOSE" then return 1
    return 0

redirectNeedsResolution: procedure = .int
    arg tokens = .token[], start_index = .int, end_index = .int
    token_count = .int
    candidate = .token
    if start_index = 0 | end_index < start_index then return 0

    token_count = 0
    do i = start_index to end_index
        ti = tokens[i]
        type = strip(ti.get_type())
        if type = "bracket" | type = "comma" then iterate
        token_count = token_count + 1
        candidate = ti
    end

    if token_count \= 1 then return 0
    if strip(candidate.get_type()) \= "identifier" then return 0
    if candidate.get_value_dims() > 0 then return 0
    if candidate.get_value_type() = ".unknown" then return 1
    return 0

emitEnvironment: procedure = .string
    arg tok = .token
    if strip(tok.get_type()) = "string_literal" then return tok.get_text()
    return quoteName(tok.get_text())

quoteName: procedure = .string
    arg name = .string
    return "'" || name || "'"

emitTokenRange: procedure = .string
    arg tokens = .token[], start_index = .int, end_index = .int
    expr = .string
    previous = .string
    expr = ""
    previous = ""
    if start_index = 0 | end_index < start_index then return ""
    do i = start_index to end_index
        text = strip(tokens[i].get_text())
        if text = "" then iterate
        if expr = "" then expr = text
        else if text = ")" | text = "]" | text = "," | text = "." then expr = expr || text
        else if previous = "(" | previous = "[" | previous = "." then expr = expr || text
        else if text = "(" | text = "[" then expr = expr || text
        else expr = expr || " " || text
        previous = text
    end
    return expr

buildRedirect: procedure = .string
    arg tokens = .token[], start_index = .int, end_index = .int, mode = .string
    expr = .string
    token_count = .int
    value_dims = .int
    expr = ""
    token_count = 0
    value_dims = 0

    if start_index = 0 then return "_noredir()"
    if end_index < start_index then return "_noredir()"

    expr = emitTokenRange(tokens, start_index, end_index)
    do i = start_index to end_index
        ti = tokens[i]
        type = strip(ti.get_type())
        if type = "bracket" | type = "comma" then iterate
        token_count = token_count + 1
        if token_count = 1 then value_dims = ti.get_value_dims()
    end

    isArray = 0
    if token_count = 1 & value_dims > 0 then isArray = 1

    if upper(mode) = "IN" then do
        if isArray then return "_array2redir(" || expr || ")"
        return "_string2redir(" || expr || ")"
    end

    if isArray then return "_redir2array(" || expr || ")"
    return "_redir2string(" || expr || ")"

buildExposeArgs: procedure = .string
    arg tokens = .token[], start_index = .int, end_index = .int
    expose_args = .string
    expose_args = ""

    if start_index = 0 | end_index < start_index then return expose_args
    do i = start_index to end_index
        ti = tokens[i]
        type = strip(ti.get_type())
        if type = "comma" then iterate
        name = strip(ti.get_text())
        expose_args = expose_args || ", '" || name || "', " || name
    end
    return expose_args
