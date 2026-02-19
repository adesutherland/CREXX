options levelb
namespace rxcp expose dumpexit
import rxcp_intern

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

    process: method = .string
        arg tokens = .token[]

        if tokens.0 < 1 then do
            _status = "REJECT"
            return _status
        end
        t1 = tokens.1

        if t1.get_text() \= "dump" then do
            _status = "REJECT"
            return _status
        end

        SAY "DUMP PLUGIN - ACCEPTED"

        /* Check types are determined - we need strings and identifiers */
        _status = "REPLACE"
        do i = 2 to tokens.0
            ti = tokens[i]
            if ti.get_type() \= "IDENTIFIER" & ti.get_type() \= "STRING_LITERAL" then do
                _status = "PENDING"
                leave
            end
        end

        if _status = "REPLACE" then do
            _replacement = ""
            do i = 2 to tokens.0
                ti = tokens[i]
                t_text = ti.get_text()
                t_type = ti.get_value_type()
                _replacement = _replacement || "say '" || t_text || " (" || t_type || ") = ' || " || t_text || ";"
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
