options levelb
namespace rxcp expose token

token: class
    _id = .int with register.1
    _type = .string with register.2
    _val_type = .string with register.3
    _text = .string with register.4
    _line = .int with register.5
    _column = .int with register.6

    *: factory
        return

    get_id: method = .int
        return _id

    get_type: method = .string
        return _type

    get_value_type: method = .string
        return _val_type

    get_text: method = .string
        return _text

    get_line: method = .int
        return _line

    get_column: method = .int
        return _column
