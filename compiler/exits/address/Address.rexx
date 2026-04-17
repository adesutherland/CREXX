options levelb
namespace rxcpexits expose addressexit
import rxcp
import rxfnsb

addressexit: class
    _node_id = .int with register.1
    _replacement = .string with register.2
    _error_token = .int with register.3
    _error_message = .string with register.4
    _status = .string with register.5

    *: factory
        arg nid = .int
        _node_id = nid
        _replacement = ""
        _error_token = 0
        _error_message = ""
        _status = "EMPTY"
        return

    get_primary_keyword: method = .string
        return "ADDRESS"

    get_additional_keywords: method = .string
        return ""

    pre_process: method = .exitplan
        arg tokens = .token[]
        _replacement = ""
        _error_token = 0
        _error_message = ""
        _status = "EMPTY"
        return .exitplan("READY")

    process: method = .string
        arg tokens = .token[]

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

        _replacement = ""
        _error_token = 0
        _error_message = ""
        _status = "EMPTY"

        if tokens.0 < 1 then return setError("ERROR", 1, "ADDRESS missing token stream")

        explicit = 0
        if upper(strip(tokens[1].get_text())) = "ADDRESS" then explicit = 1

        if explicit = 1 then do
            if tokens.0 < 3 then return setError("ERROR", 1, "ADDRESS requires an environment and command")
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
                    if input_start > 0 then return setError("ERROR", i, "ADDRESS INPUT clause repeated")
                    input_start = i + 1
                end
                else if utext = "OUTPUT" then do
                    if output_start > 0 then return setError("ERROR", i, "ADDRESS OUTPUT clause repeated")
                    output_start = i + 1
                end
                else if utext = "ERROR" then do
                    if error_start > 0 then return setError("ERROR", i, "ADDRESS ERROR clause repeated")
                    error_start = i + 1
                end
                else if utext = "EXPOSE" then do
                    if expose_start > 0 then return setError("ERROR", i, "ADDRESS EXPOSE clause repeated")
                    expose_start = i + 1
                end
                section = utext
                if i = tokens.0 then return setError("ERROR", i, "ADDRESS clause requires an operand")
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

        if command_end < command_start then return setError("ERROR", clause_start, "ADDRESS requires a command expression")
        if input_start > 0 & input_end < input_start then return setError("ERROR", input_start - 1, "ADDRESS INPUT clause requires an operand")
        if output_start > 0 & output_end < output_start then return setError("ERROR", output_start - 1, "ADDRESS OUTPUT clause requires an operand")
        if error_start > 0 & error_end < error_start then return setError("ERROR", error_start - 1, "ADDRESS ERROR clause requires an operand")
        if expose_start > 0 & expose_end < expose_start then return setError("ERROR", expose_start - 1, "ADDRESS EXPOSE clause requires an operand")
        if redirectNeedsResolution(tokens, input_start, input_end) then return setPending()
        if redirectNeedsResolution(tokens, output_start, output_end) then return setPending()
        if redirectNeedsResolution(tokens, error_start, error_end) then return setPending()

        _replacement = "rc=_address(" || env_expr
        _replacement = _replacement || "," || emitTokenRange(tokens, command_start, command_end)
        _replacement = _replacement || "," || buildRedirect(tokens, input_start, input_end, "IN")
        _replacement = _replacement || "," || buildRedirect(tokens, output_start, output_end, "OUT")
        _replacement = _replacement || "," || buildRedirect(tokens, error_start, error_end, "OUT")
        _replacement = _replacement || buildExposeArgs(tokens, expose_start, expose_end)
        _replacement = _replacement || ")"

        _status = "REPLACE"
        return _status

    get_replacement: method = .string
        return _replacement

    get_error_token: method = .int
        return _error_token

    get_error_message: method = .string
        return _error_message

    get_status: method = .string
        return _status

    get_node_id: method = .int
        return _node_id

setPending: procedure = .string
    _status = "PENDING"
    return _status

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
        text = strip(ti.get_text())
        if type \= "identifier" then iterate
        expose_args = expose_args || ", " || quoteName(text) || ", " || text
    end
    return expose_args

setError: procedure = .string
    arg status = "EMPTY", error_token = 0, error_message = "unknown"
    _status = status
    _error_token = error_token
    _error_message = error_message
    return _status
