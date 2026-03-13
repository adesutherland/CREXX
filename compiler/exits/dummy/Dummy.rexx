options levelb
namespace rxcpexits expose dummyexit
import rxcp

dummyexit: class
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
        return "dummy"

    get_additional_keywords: method = .string
        return ""

    pre_process: method = .string
        arg tokens = .token[]

        if tokens.0 < 2 then return ""

        /* Tell the compiler we want to hoist the variable at token index 2 */
        return "2"

    process: method = .string
        arg tokens = .token[]

        /* Require exactly one identifier argument to shadow */
        if tokens.0 \= 2 then do
            _error_token = 1
            _error_message = "INVALID_ARGUMENTS"
            _status = "ERROR"
            return _status
        end

        /* Only accept identifiers */
        ti = tokens[2]
        if ti.get_type() \= "identifier" then do
            _status = "ERROR"
            _error_token = 2
            _error_message = "Not an identifier"
            return _status
        end

        /* Wait until the identifier has a known type to avoid premature lowering */
        if ti.get_value_type() = ".unknown" then do
            _status = "PENDING"
            return _status
        end

        /* Generate a replacement that declares a typed local with the same name
           as the argument and assigns/prints it. This will be wrapped by the
           compiler in an EXIT_OWNED block, so the declaration must not leak. */
        name = .string
        name = ti.get_text()
        _replacement = "say '" || name || " (" || ti.get_value_type() || ") = ' || {" || 2 || "}; " || name || " = .int; " || name || " = 1; say 'in-exit ' || " || name || "; dummy_var = 'hoisted_ok'; "
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
