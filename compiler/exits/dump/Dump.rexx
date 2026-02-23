options levelb
namespace rxcpexits expose dumpexit
import rxcp
import rxfnsb

dumpexit: class
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

    get_primary_keyword: method = .string
        return "dump"

    get_additional_keywords: method = .string
        return ""

    process: method = .string
        arg tokens = .token[]

        if tokens.0 < 1 then do
            _status = "REJECT"
            return _status
        end

        /* Check Errors */
        do i = 2 to tokens.0
            ti = tokens[i]
            if ti.get_type() \= "identifier" then do
                _status = "ERROR"
                _error_token = i
                _error_message = "Not an identifier"
                return _status
            end
        end

        /* Check types are determined */
        _status = "REPLACE"
        do i = 2 to tokens.0
            ti = tokens[i]
            if ti.get_value_type() = ".unknown" then do
                _status = "PENDING"
                return _status
            end
        end

        if _status = "REPLACE" then do
            _replacement = ""
            do i = 2 to tokens.0
                ti = tokens[i]
                t_text = ti.get_text()
                t_type = ti.get_value_type()
                /* If the token is an array, loop through its elements and print each one */
                if pos("[", t_type) > 0 then do
                    _replacement = _replacement || "__rxcpx_dump_i = 1; do while __rxcpx_dump_i <= {" || i || "}[0]; say '" || t_text || " (" || t_type || ")[" || "' || __rxcpx_dump_i || '" || "] = ' || {" || i || "}[__rxcpx_dump_i]; __rxcpx_dump_i = __rxcpx_dump_i + 1; end;"
                end
                else do
                    _replacement = _replacement || "say '" || t_text || " (" || t_type || ") = ' || {" || i || "};"
                end
            end
        end

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
