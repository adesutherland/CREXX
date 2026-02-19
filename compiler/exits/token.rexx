options levelb
namespace rxcp_intern expose token

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
        arg t=.int, st=.int, txt=.string, l=.int, c=.int, len=.int, f=.string, nt=.int, vt=.int
        _type = t
        _subtype = st
        _text = txt
        _line = l
        _column = c
        _length = len
        _file = f
        _node_type = nt
        _value_type = vt
        return

    get_id: method = .int
        return 0 /* Not used for now */

    get_type_int: method = .int
        return _type

    get_type: method = .string
        /* Todo: convert int type to string if needed, or just return int */
        /* These values should match TK_ constants in the compiler */
        if _type = 14 then return "IDENTIFIER"
        if _type = 27 then return "STRING_LITERAL"
        if _type = 28 then return "INT_LITERAL"
        if _type = 29 then return "FLOAT_LITERAL"
        if _type = 30 then return "DECIMAL_LITERAL"
        return "OTHER"

    get_value_type_int: method = .int
        return _value_type

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

    get_length: method = .int
        return _length

    get_file: method = .string
        return _file

    get_node_type: method = .int
        return _node_type

    set_type: method
        arg t = .int
        _type = t

    set_text: method
        arg t = .string
        _text = t
