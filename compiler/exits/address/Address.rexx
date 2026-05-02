options levelb
namespace rxcpexits expose addressexit

import rxcp
import rxfnsb

addressexit: class
    _node_id = .int

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
        sandbox_start = .int
        sandbox_end = .int
        auto_expose = .string[]
        section = .string
        depth = .int

        if tokens.0 < 1 then return error_result(1, "ADDRESS missing token stream")

        explicit = 0
        if upper(strip(tokens[1].get_text())) = "ADDRESS" then explicit = 1

        if explicit = 1 then do
            if tokens.0 < 2 then return error_result(1, "ADDRESS requires an environment or command")
            if strip(tokens[2].get_type()) = "string_literal" then do
                env_expr = "''"
                clause_start = 2
            end
            else do
                env_expr = emitEnvironment(tokens[2])
                if tokens.0 = 2 then return set_environment_result(env_expr)
                clause_start = 3
            end
        end
        else do
            env_expr = "''"
            clause_start = 1
            call maybe_add_implicit_warning(result, tokens)
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
        sandbox_start = 0
        sandbox_end = -1
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
                else if section = "SANDBOX" then sandbox_end = i - 1
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
                else if utext = "SANDBOX" then do
                    if sandbox_start > 0 then return error_result(i, "ADDRESS SANDBOX clause repeated")
                    sandbox_start = i + 1
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
        else if section = "SANDBOX" then sandbox_end = tokens.0

        if command_end < command_start then do
            if explicit = 1 & sandbox_start > 0 & input_start = 0 & output_start = 0 & error_start = 0 & expose_start = 0 then do
                if sandbox_end < sandbox_start then return error_result(sandbox_start - 1, "ADDRESS SANDBOX clause requires an operand")
                return error_result(sandbox_start - 1, "ADDRESS SANDBOX must be attached to a command")
            end
            return error_result(clause_start, "ADDRESS requires a command expression")
        end
        if input_start > 0 & input_end < input_start then return error_result(input_start - 1, "ADDRESS INPUT clause requires an operand")
        if output_start > 0 & output_end < output_start then return error_result(output_start - 1, "ADDRESS OUTPUT clause requires an operand")
        if error_start > 0 & error_end < error_start then return error_result(error_start - 1, "ADDRESS ERROR clause requires an operand")
        if expose_start > 0 & expose_end < expose_start then return error_result(expose_start - 1, "ADDRESS EXPOSE clause requires an operand")
        if sandbox_start > 0 & sandbox_end < sandbox_start then return error_result(sandbox_start - 1, "ADDRESS SANDBOX clause requires an operand")
        if redirectNeedsResolution(tokens, input_start, input_end) then return pending_result()
        if redirectNeedsResolution(tokens, output_start, output_end) then return pending_result()
        if redirectNeedsResolution(tokens, error_start, error_end) then return pending_result()

        auto_expose = collectHostVariableAnchors(tokens, command_start, command_end, expose_start, expose_end)

        i = expose_start
        do while i <= expose_end
            if i = 0 then leave
            ti = tokens[i]
            type = strip(ti.get_type())
            if type = "comma" then do
                i = i + 1
                iterate
            end
            if type \= "identifier" then return error_result(i, "ADDRESS EXPOSE only accepts variable names")
            if isExposeArrayAt(tokens, i, expose_end) then do
                if strip(ti.get_value_type()) \= ".unknown" & left(strip(ti.get_value_type()), 7) \= ".string" then return error_result(i, "ADDRESS EXPOSE array bindings must be .string[]")
            end
            i = nextExposeIndex(tokens, i, expose_end)
        end

        if exposeNeedsGeneratedRequest(tokens, expose_start, expose_end) | auto_expose.0 > 0 then do
            return emitGeneratedRequestResult(result, tokens, env_expr, command_start, command_end, input_start, input_end, output_start, output_end, error_start, error_end, expose_start, expose_end, sandbox_start, sandbox_end, auto_expose)
        end

        if sandbox_start > 0 then replacement = "rc=_address_with_sandbox(" || env_expr
        else replacement = "rc=_address(" || env_expr
        replacement = replacement || "," || emitTokenRange(tokens, command_start, command_end)
        replacement = replacement || "," || buildRedirect(tokens, input_start, input_end, "IN")
        replacement = replacement || "," || buildRedirect(tokens, output_start, output_end, "OUT")
        replacement = replacement || "," || buildRedirect(tokens, error_start, error_end, "OUT")
        if sandbox_start > 0 then replacement = replacement || "," || emitTokenRange(tokens, sandbox_start, sandbox_end)
        replacement = replacement || buildExposeArgs(tokens, expose_start, expose_end)
        replacement = replacement || buildAutoExposeArgs(auto_expose)
        replacement = replacement || ")"

        call result.set_status("REPLACE")
        call result.add_replacement_line(replacement)
        return result

set_environment_result: procedure = .exitresult
    arg env_expr = .string
    result = .exitresult("REPLACE")
    call result.add_replacement_line("rc=_set_address_environment(" || env_expr || ")")
    return result

pending_result: procedure = .exitresult
    result = .exitresult("PENDING")
    return result

error_result: procedure = .exitresult
    arg token_index = .int, message = .string
    result = .exitresult("ERROR")
    call result.set_error(token_index, message)
    return result

maybe_add_implicit_warning: procedure = .void
    arg result = .exitresult, tokens = .token[]
    text = .string

    if tokens.0 < 1 then return
    text = tokens[1].get_text()
    if length(text) > 0 & (left(text, 1) = "'" | left(text, 1) = '"') then return

    call result.add_diagnostic(.exitdiagnostic("warning", 1, "IMPLICIT_ADDRESS", ""))
    return

isClauseKeyword: procedure = .int
    arg text = .string
    if text = "INPUT" | text = "OUTPUT" | text = "ERROR" | text = "EXPOSE" | text = "SANDBOX" then return 1
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
    join_before = .string
    expr = ""
    previous = ""
    if start_index = 0 | end_index < start_index then return ""
    do i = start_index to end_index
        text = strip(tokens[i].get_text())
        join_before = upper(strip(tokens[i].get_join_before()))
        if text = "" then iterate
        if expr = "" then expr = text
        else if join_before = "CONCAT" then expr = expr || text
        else if join_before = "SCONCAT" then expr = expr || " " || text
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

isExposeArrayToken: procedure = .int
    arg tok = .token
    if tok.get_value_dims() > 0 then return 1
    if pos("[", strip(tok.get_value_type())) > 0 then return 1
    return 0

isExposeArrayAt: procedure = .int
    arg tokens = .token[], start_index = .int, end_index = .int
    open_tok = .token
    close_tok = .token

    if start_index = 0 | start_index > end_index then return 0
    if isExposeArrayToken(tokens[start_index]) then return 1

    if start_index + 2 > end_index then return 0
    open_tok = tokens[start_index + 1]
    close_tok = tokens[start_index + 2]
    if strip(open_tok.get_type()) = "bracket" & strip(open_tok.get_text()) = "[" & strip(close_tok.get_type()) = "bracket" & strip(close_tok.get_text()) = "]" then return 1
    return 0

nextExposeIndex: procedure = .int
    arg tokens = .token[], start_index = .int, end_index = .int
    if isExposeArrayAt(tokens, start_index, end_index) & start_index + 2 <= end_index then return start_index + 3
    return start_index + 1

exposeNeedsGeneratedRequest: procedure = .int
    arg tokens = .token[], start_index = .int, end_index = .int
    if start_index = 0 | end_index < start_index then return 0

    i = start_index
    do while i <= end_index
        ti = tokens[i]
        type = strip(ti.get_type())
        if type = "comma" then do
            i = i + 1
            iterate
        end
        if isExposeArrayAt(tokens, i, end_index) then return 1
        i = nextExposeIndex(tokens, i, end_index)
    end
    return 0

emitGeneratedRequestResult: procedure = .exitresult
    arg result = .exitresult, tokens = .token[], env_expr = .string, command_start = .int, command_end = .int, input_start = .int, input_end = .int, output_start = .int, output_end = .int, error_start = .int, error_end = .int, expose_start = .int, expose_end = .int, sandbox_start = .int, sandbox_end = .int, auto_expose = .string[]

    request_var = "__rxaddr_request"
    response_var = "__rxaddr_response"
    command_expr = .string
    input_expr = .string
    output_expr = .string
    error_expr = .string
    sandbox_expr = .string
    stem_index = .int
    binding_index = .int

    command_expr = emitTokenRange(tokens, command_start, command_end)
    input_expr = buildRedirect(tokens, input_start, input_end, "IN")
    output_expr = buildRedirect(tokens, output_start, output_end, "OUT")
    error_expr = buildRedirect(tokens, error_start, error_end, "OUT")

    call result.set_status("REPLACE")
    call result.add_replacement_line(request_var || "=_address_new_request(" || env_expr || "," || command_expr || "," || input_expr || "," || output_expr || "," || error_expr || ")")

    if sandbox_start > 0 then do
        sandbox_expr = emitTokenRange(tokens, sandbox_start, sandbox_end)
        call result.add_replacement_line("call _address_link_request_sandbox(" || request_var || "," || sandbox_expr || ")")
    end

    stem_index = 0
    binding_index = 0
    i = expose_start
    do while i <= expose_end
        if i = 0 then leave
        ti = tokens[i]
        type = strip(ti.get_type())
        if type = "comma" then do
            i = i + 1
            iterate
        end

        name = strip(ti.get_text())
        binding_index = binding_index + 1
        if isExposeArrayAt(tokens, i, expose_end) then do
            stem_index = stem_index + 1
            stem_var = "__rxaddr_stem_" || stem_index
            loop_var = "__rxaddr_i_" || stem_index
            binding_var = "__rxaddr_binding_" || binding_index
            call result.add_replacement_line(stem_var || "=.standardaddressstem()")
            call result.add_replacement_line("call " || stem_var || ".set('0'," || name || ".0 || '')")
            call result.add_replacement_line("do " || loop_var || " = 1 to " || name || ".0")
            call result.add_replacement_line("call " || stem_var || ".set(" || loop_var || " || ''," || name || "[" || loop_var || "])")
            call result.add_replacement_line("end")
            call result.add_replacement_line(binding_var || "=.addressbinding('stem','" || name || "','" || name || "','','')")
            call result.add_replacement_line("assembler linktoattr1 6, " || binding_var || ", " || stem_var)
            call result.add_replacement_line("call " || request_var || ".add_binding_plan(" || binding_var || ")")
        end
        else do
            binding_var = "__rxaddr_binding_" || binding_index
            call result.add_replacement_line(binding_var || "=.addressbinding('var','" || name || "','" || name || "'," || name || ",'')")
            call result.add_replacement_line("call " || request_var || ".add_binding_plan(" || binding_var || ")")
        end
        i = nextExposeIndex(tokens, i, expose_end)
    end

    do auto_index = 1 to auto_expose.0
        name = auto_expose[auto_index]
        binding_index = binding_index + 1
        binding_var = "__rxaddr_binding_" || binding_index
        call result.add_replacement_line(binding_var || "=.addressbinding('var','" || name || "','" || name || "'," || name || ",'')")
        call result.add_replacement_line("call " || request_var || ".add_binding_plan(" || binding_var || ")")
    end

    call result.add_replacement_line(response_var || "=_address_dispatch_request(" || request_var || ")")

    if sandbox_start > 0 then do
        call result.add_replacement_line("call _address_apply_response_sandbox(" || response_var || "," || sandbox_expr || ")")
    end

    stem_index = 0
    binding_index = 0
    i = expose_start
    do while i <= expose_end
        if i = 0 then leave
        ti = tokens[i]
        type = strip(ti.get_type())
        if type = "comma" then do
            i = i + 1
            iterate
        end

        name = strip(ti.get_text())
        binding_index = binding_index + 1
        if isExposeArrayAt(tokens, i, expose_end) then do
            stem_index = stem_index + 1
            stem_var = "__rxaddr_stem_" || stem_index
            loop_var = "__rxaddr_i_" || stem_index
            count_var = "__rxaddr_count_" || stem_index
            call result.add_replacement_line("call _address_apply_response_request_stem(" || response_var || "," || request_var || "," || binding_index || ",'" || name || "')")
            call result.add_replacement_line(count_var || "=" || request_var || ".get_binding_stem_value(" || binding_index || ",'0') + 0")
            call result.add_replacement_line("assembler SETATTRS " || name || "," || count_var)
            call result.add_replacement_line("do " || loop_var || " = 1 to " || count_var)
            call result.add_replacement_line(name || "[" || loop_var || "]=" || request_var || ".get_binding_stem_value(" || binding_index || "," || loop_var || " || '')")
            call result.add_replacement_line("end")
        end
        else do
            call result.add_replacement_line("call _address_apply_response_var(" || response_var || ",'" || name || "'," || name || ")")
        end
        i = nextExposeIndex(tokens, i, expose_end)
    end

    do auto_index = 1 to auto_expose.0
        name = auto_expose[auto_index]
        call result.add_replacement_line("call _address_apply_response_var(" || response_var || ",'" || name || "'," || name || ")")
    end

    call result.add_replacement_line("rc=" || response_var || ".get_rc()")
    return result

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

buildAutoExposeArgs: procedure = .string
    arg auto_expose = .string[]
    expose_args = .string
    expose_args = ""

    do i = 1 to auto_expose.0
        name = auto_expose[i]
        expose_args = expose_args || ", '" || name || "', " || name
    end
    return expose_args

collectHostVariableAnchors: procedure = .string[]
    arg tokens = .token[], start_index = .int, end_index = .int, expose_start = .int, expose_end = .int
    anchors = .string[]
    if start_index = 0 | end_index < start_index then return anchors

    do i = start_index to end_index
        ti = tokens[i]
        type = strip(ti.get_type())
        text = strip(ti.get_text())
        if type = "string_literal" then call scanHostAnchorsInText anchors, text, tokens, expose_start, expose_end, 1
        else call scanHostAnchorsInText anchors, text, tokens, expose_start, expose_end, 0
    end

    return anchors

scanHostAnchorsInText: procedure = .void
    arg expose anchors = .string[], text = .string, tokens = .token[], expose_start = .int, expose_end = .int, allow_colon = .int
    text_len = .int
    ch = .string
    next_ch = .string
    name = .string
    close_pos = .int
    j = .int

    text_len = length(text)
    i = 1
    do while i <= text_len
        ch = substr(text, i, 1)

        if ch = "$" & i < text_len & substr(text, i + 1, 1) = "{" then do
            close_pos = pos("}", text, i + 2)
            if close_pos > i + 2 then do
                name = substr(text, i + 2, close_pos - i - 2)
                if isHostAnchorName(name) = 1 then call addHostAnchor anchors, name, tokens, expose_start, expose_end
                i = close_pos + 1
                iterate
            end
        end

        if allow_colon = 1 & ch = ":" then do
            if i > 1 & substr(text, i - 1, 1) = ":" then do
                i = i + 1
                iterate
            end
            if i < text_len then next_ch = substr(text, i + 1, 1)
            else next_ch = ""
            if isHostAnchorStart(next_ch) = 1 then do
                j = i + 1
                do while j <= text_len & isHostAnchorPart(substr(text, j, 1)) = 1
                    j = j + 1
                end
                name = substr(text, i + 1, j - i - 1)
                call addHostAnchor anchors, name, tokens, expose_start, expose_end
                i = j
                iterate
            end
        end

        i = i + 1
    end
    return

addHostAnchor: procedure = .void
    arg expose anchors = .string[], name = .string, tokens = .token[], expose_start = .int, expose_end = .int
    normalized = .string

    if isHostAnchorName(name) \= 1 then return
    if explicitExposeContains(tokens, expose_start, expose_end, name) = 1 then return

    normalized = upper(name)
    do i = 1 to anchors.0
        if upper(anchors[i]) = normalized then return
    end

    anchors[anchors.0 + 1] = name
    return

explicitExposeContains: procedure = .int
    arg tokens = .token[], start_index = .int, end_index = .int, name = .string
    normalized = .string

    if start_index = 0 | end_index < start_index then return 0
    normalized = upper(name)
    i = start_index
    do while i <= end_index
        ti = tokens[i]
        type = strip(ti.get_type())
        if type = "comma" then do
            i = i + 1
            iterate
        end
        if type = "identifier" & upper(strip(ti.get_text())) = normalized then return 1
        i = nextExposeIndex(tokens, i, end_index)
    end
    return 0

isHostAnchorName: procedure = .int
    arg name = .string
    if length(name) < 1 then return 0
    if isHostAnchorStart(substr(name, 1, 1)) \= 1 then return 0
    do i = 2 to length(name)
        if isHostAnchorPart(substr(name, i, 1)) \= 1 then return 0
    end
    return 1

isHostAnchorStart: procedure = .int
    arg ch = .string
    if ch = "_" then return 1
    if pos(upper(ch), "ABCDEFGHIJKLMNOPQRSTUVWXYZ") > 0 then return 1
    return 0

isHostAnchorPart: procedure = .int
    arg ch = .string
    if isHostAnchorStart(ch) = 1 then return 1
    if pos(ch, "0123456789") > 0 then return 1
    return 0
