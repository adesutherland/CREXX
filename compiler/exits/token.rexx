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
    _value_type = .string with register.13
    _type_string = .string with register.14

    *: factory
        arg t=.int, st=.int, txt=.string, l=.int, c=.int, len=.int, f=.string, nt=.int, vt=.string, ts=.string
        _type = t
        _subtype = st
        _text = txt
        _line = l
        _column = c
        _length = len
        _file = f
        _node_type = nt
        _value_type = vt
        _type_string = ts
        return

    get_id: method = .int
        return 0 /* Not used for now */

    get_type_int: method = .int
        return _type

    get_type: method = .string
        return _type_string


    get_value_type: method = .string
        return _value_type

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
