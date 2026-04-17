options levelb
namespace rxcp expose token exitplan bindingplan keywordclaim exitresult

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
    _value_dims = .int with register.15

    *: factory
        arg t=.int, st=.int, txt=.string, l=.int, c=.int, len=.int, f=.string, nt=.int, vt=.string, ts=.string, vd=.int
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
        _value_dims = vd
        return

    get_id: method = .int
        return 0 /* Not used for now */

    get_type_int: method = .int
        return _type

    get_type: method = .string
        return _type_string


    get_value_type: method = .string
        return _value_type

    get_value_dims: method = .int
        return _value_dims

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

exitplan: class
    _status = .string with register.1
    _binding_count = .int with register.2
    _binding_kind = .string[] with register.3
    _binding_internal_name = .string[] with register.4
    _binding_external_alias = .string[] with register.5
    _binding_value_type = .string[] with register.6
    _binding_dimensions = .int[] with register.7
    _binding_provenance = .string[] with register.8
    _binding_flags = .string[] with register.9
    _keyword_count = .int with register.10
    _keyword_token_index = .int[] with register.11
    _keyword_text = .string[] with register.12
    _keyword_role = .string[] with register.13
    _keyword_provenance = .string[] with register.14
    _error_token = .int with register.15
    _error_message = .string with register.16

    *: factory
        arg status = .string
        if status = "" then status = "EMPTY"
        _status = status
        _binding_count = 0
        _keyword_count = 0
        _error_token = 0
        _error_message = ""
        return

    get_status: method = .string
        return _status

    set_status: method
        arg status = .string
        _status = status

    add_binding: method
        arg kind = .string, internal_name = .string, external_alias = .string, value_type = .string, dimensions = .int, provenance = .string, flags = .string
        i = .int
        i = _binding_count + 1
        _binding_count = i
        _binding_kind[i] = kind
        _binding_internal_name[i] = internal_name
        _binding_external_alias[i] = external_alias
        _binding_value_type[i] = value_type
        _binding_dimensions[i] = dimensions
        _binding_provenance[i] = provenance
        _binding_flags[i] = flags

    get_binding_count: method = .int
        return _binding_count

    get_binding_kind: method = .string
        arg index = .int
        return _binding_kind[index]

    get_binding_internal_name: method = .string
        arg index = .int
        return _binding_internal_name[index]

    get_binding_external_alias: method = .string
        arg index = .int
        return _binding_external_alias[index]

    get_binding_value_type: method = .string
        arg index = .int
        return _binding_value_type[index]

    get_binding_dimensions: method = .int
        arg index = .int
        return _binding_dimensions[index]

    get_binding_provenance: method = .string
        arg index = .int
        return _binding_provenance[index]

    get_binding_flags: method = .string
        arg index = .int
        return _binding_flags[index]

    add_keyword: method
        arg token_index = .int, keyword_text = .string, keyword_role = .string, provenance = .string
        i = .int
        i = _keyword_count + 1
        _keyword_count = i
        _keyword_token_index[i] = token_index
        _keyword_text[i] = keyword_text
        _keyword_role[i] = keyword_role
        _keyword_provenance[i] = provenance

    get_keyword_count: method = .int
        return _keyword_count

    get_keyword_token_index: method = .int
        arg index = .int
        return _keyword_token_index[index]

    get_keyword_text: method = .string
        arg index = .int
        return _keyword_text[index]

    get_keyword_role: method = .string
        arg index = .int
        return _keyword_role[index]

    get_keyword_provenance: method = .string
        arg index = .int
        return _keyword_provenance[index]

    set_error: method
        arg token_index = .int, message = .string
        _status = "ERROR"
        _error_token = token_index
        _error_message = message

    get_error_token: method = .int
        return _error_token

    get_error_message: method = .string
        return _error_message

bindingplan: class
    _kind = .string with register.1
    _internal_name = .string with register.2
    _external_alias = .string with register.3
    _value_type = .string with register.4
    _dimensions = .int with register.5
    _provenance = .string with register.6
    _flags = .string with register.7

    *: factory
        arg kind = .string, internal_name = .string, external_alias = .string, value_type = .string, dimensions = .int, provenance = .string, flags = .string
        _kind = kind
        _internal_name = internal_name
        _external_alias = external_alias
        _value_type = value_type
        _dimensions = dimensions
        _provenance = provenance
        _flags = flags
        return

    get_kind: method = .string
        return _kind

    get_internal_name: method = .string
        return _internal_name

    get_external_alias: method = .string
        return _external_alias

    get_value_type: method = .string
        return _value_type

    get_dimensions: method = .int
        return _dimensions

    get_provenance: method = .string
        return _provenance

    get_flags: method = .string
        return _flags

keywordclaim: class
    _token_index = .int with register.1
    _keyword_text = .string with register.2
    _keyword_role = .string with register.3
    _provenance = .string with register.4

    *: factory
        arg token_index = .int, keyword_text = .string, keyword_role = .string, provenance = .string
        _token_index = token_index
        _keyword_text = keyword_text
        _keyword_role = keyword_role
        _provenance = provenance
        return

    get_token_index: method = .int
        return _token_index

    get_keyword_text: method = .string
        return _keyword_text

    get_keyword_role: method = .string
        return _keyword_role

    get_provenance: method = .string
        return _provenance

exitresult: class
    _status = .string with register.1
    _replacement = .string with register.2
    _error_token = .int with register.3
    _error_message = .string with register.4

    *: factory
        arg status = .string
        if status = "" then status = "EMPTY"
        _status = status
        _replacement = ""
        _error_token = 0
        _error_message = ""
        return

    get_status: method = .string
        return _status

    set_status: method
        arg status = .string
        _status = status

    get_replacement: method = .string
        return _replacement

    set_replacement: method
        arg replacement = .string
        _replacement = replacement

    get_error_token: method = .int
        return _error_token

    get_error_message: method = .string
        return _error_message

    set_error: method
        arg token_index = .int, message = .string
        _status = "ERROR"
        _error_token = token_index
        _error_message = message
