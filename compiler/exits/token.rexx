options levelb
namespace rxcp expose token

token: class
    _type = .int with register.1
    _subtype = .int with register.2
    _text = .string with register.3
    _line = .int with register.4
    _column = .int with register.5
    _length = .int with register.6
    _file = .string with register.7
    _node_type = .int with register.8
    _value_type = .int with register.13

    *: factory
        return

    get_id: method = .int
        return 0 /* Not used for now */

    get_type: method = .string
        /* Todo: convert int type to string if needed, or just return int */
        if _type = 14 then return "IDENTIFIER"
        if _type = 27 then return "STRING_LITERAL"
        return "OTHER"

    get_value_type: method = .string
        if _value_type = 0 then return "UNKNOWN"
        if _value_type = 1 then return "VOID"
        if _value_type = 2 then return "BOOLEAN"
        if _value_type = 3 then return "INTEGER"
        if _value_type = 4 then return "FLOAT"
        if _value_type = 5 then return "DECIMAL"
        if _value_type = 6 then return "STRING"
        if _value_type = 7 then return "BINARY"
        if _value_type = 8 then return "OBJECT"
        return "UNKNOWN"

    get_text: method = .string
        return _text

    get_line: method = .int
        return _line

    get_column: method = .int
        return _column
