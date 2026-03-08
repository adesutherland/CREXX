options levelb
namespace rxcpexits expose queryexit
import rxcp
import rxfnsb

queryexit: class
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
        return "query"

    get_additional_keywords: method = .string
        return "crexx os"

    process: method = .string
        arg tokens = .token[]

        if tokens.0 < 2 then do
            _status = "ERROR"
            _error_token = 1
            _error_message = "Missing query argument (crexx or os)"
            return _status
        end

        arg1 = tokens[2].get_text()
        
        if arg1 = "crexx" then do
            _replacement = "do; _q_info = ''; assembler rxvers _q_info; say 'cREXX Version ' || word(_q_info, 3); end;"
            _status = "REPLACE"
        end
        else if arg1 = "os" then do
            _replacement = "do; _q_info = ''; assembler rxvers _q_info; say 'OS: ' || word(_q_info, 1); end;"
            _status = "REPLACE"
        end
        else do
            _status = "ERROR"
            _error_token = 2
            _error_message = "Invalid query argument: " || arg1
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
