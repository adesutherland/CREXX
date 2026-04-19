options levelb
namespace rxcp expose token exitdescriptor exitplan bindingplan keywordclaim importplan helperplan exitdiagnostic exitresult

token: class
    _type = .int
    _subtype = .int
    _text = .string
    _line = .int
    _column = .int
    _length = .int
    _file = .string
    _node_type = .int
    _value_type = .string
    _type_string = .string
    _value_dims = .int

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
        return 0

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

    set_type: method = .void
        arg t = .int
        _type = t

    set_text: method = .void
        arg t = .string
        _text = t

bindingplan: class
    _kind = .string
    _internal_name = .string
    _external_alias = .string
    _value_type = .string
    _dimensions = .int
    _provenance = .string
    _flags = .string

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
    _token_index = .int
    _keyword_text = .string
    _keyword_role = .string
    _provenance = .string

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

importplan: class
    _namespace_name = .string
    _provenance = .string
    _flags = .string

    *: factory
        arg namespace_name = .string, provenance = .string, flags = .string
        _namespace_name = namespace_name
        _provenance = provenance
        _flags = flags
        return

    get_namespace_name: method = .string
        return _namespace_name

    get_provenance: method = .string
        return _provenance

    get_flags: method = .string
        return _flags

helperplan: class
    _helper_id = .string
    _scope = .string
    _symbol_name = .string
    _flags = .string
    _line_count = .int
    _lines = .string[]

    *: factory
        arg helper_id = .string, scope = .string, symbol_name = .string, flags = .string
        _helper_id = helper_id
        _scope = scope
        _symbol_name = symbol_name
        _flags = flags
        _line_count = 0
        return

    add_line: method = .void
        arg line = .string
        i = .int
        i = _line_count + 1
        _line_count = i
        _lines[i] = line

    get_helper_id: method = .string
        return _helper_id

    get_scope: method = .string
        return _scope

    get_symbol_name: method = .string
        return _symbol_name

    get_flags: method = .string
        return _flags

    get_line_count: method = .int
        return _line_count

    get_line: method = .string
        arg index = .int
        return _lines[index]

exitdiagnostic: class
    _severity = .string
    _token_index = .int
    _code = .string
    _message = .string

    *: factory
        arg severity = .string, token_index = .int, code = .string, message = .string
        _severity = severity
        _token_index = token_index
        _code = code
        _message = message
        return

    get_severity: method = .string
        return _severity

    get_token_index: method = .int
        return _token_index

    get_code: method = .string
        return _code

    get_message: method = .string
        return _message

exitdescriptor: class
    _protocol_version = .int
    _primary_keyword = .string
    _additional_keyword_count = .int
    _additional_keywords = .string[]
    _flag_count = .int
    _flags = .string[]
    _import_count = .int
    _imports = .importplan[]

    *: factory
        arg primary_keyword = .string
        _protocol_version = 2
        _primary_keyword = primary_keyword
        _additional_keyword_count = 0
        _flag_count = 0
        _import_count = 0
        return

    get_protocol_version: method = .int
        return _protocol_version

    get_primary_keyword: method = .string
        return _primary_keyword

    add_additional_keyword: method = .void
        arg keyword = .string
        i = .int
        i = _additional_keyword_count + 1
        _additional_keyword_count = i
        _additional_keywords[i] = keyword

    get_additional_keyword_count: method = .int
        return _additional_keyword_count

    get_additional_keyword: method = .string
        arg index = .int
        return _additional_keywords[index]

    add_flag: method = .void
        arg flag = .string
        i = .int
        i = _flag_count + 1
        _flag_count = i
        _flags[i] = flag

    get_flag_count: method = .int
        return _flag_count

    get_flag: method = .string
        arg index = .int
        return _flags[index]

    has_flag: method = .int
        arg flag = .string
        do i = 1 to _flag_count
            if _flags[i] = flag then return 1
        end
        return 0

    add_import: method = .void
        arg namespace_name = .string, provenance = .string, flags = .string
        call add_import_plan(.importplan(namespace_name, provenance, flags))

    add_import_plan: method = .void
        arg plan = .importplan
        i = .int
        i = _import_count + 1
        _import_count = i
        _imports[i] = plan

    get_default_import_count: method = .int
        return _import_count

    get_default_import: method = .importplan
        arg index = .int
        return _imports[index]

exitplan: class
    _status = .string
    _binding_count = .int
    _bindings = .bindingplan[]
    _keyword_count = .int
    _keywords = .keywordclaim[]
    _import_count = .int
    _imports = .importplan[]
    _helper_count = .int
    _helpers = .helperplan[]
    _diagnostic_count = .int
    _diagnostics = .exitdiagnostic[]
    _note_count = .int
    _notes = .string[]

    *: factory
        arg status = .string
        if status = "" then status = "EMPTY"
        _status = status
        _binding_count = 0
        _keyword_count = 0
        _import_count = 0
        _helper_count = 0
        _diagnostic_count = 0
        _note_count = 0
        return

    get_status: method = .string
        return _status

    set_status: method = .void
        arg status = .string
        _status = status

    add_binding: method = .void
        arg kind = .string, internal_name = .string, external_alias = .string, value_type = .string, dimensions = .int, provenance = .string, flags = .string
        call add_binding_plan(.bindingplan(kind, internal_name, external_alias, value_type, dimensions, provenance, flags))

    add_binding_plan: method = .void
        arg plan = .bindingplan
        i = .int
        i = _binding_count + 1
        _binding_count = i
        _bindings[i] = plan

    get_binding_count: method = .int
        return _binding_count

    get_binding: method = .bindingplan
        arg index = .int
        return _bindings[index]

    add_keyword: method = .void
        arg token_index = .int, keyword_text = .string, keyword_role = .string, provenance = .string
        call add_keyword_claim(.keywordclaim(token_index, keyword_text, keyword_role, provenance))

    add_keyword_claim: method = .void
        arg claim = .keywordclaim
        i = .int
        i = _keyword_count + 1
        _keyword_count = i
        _keywords[i] = claim

    get_keyword_count: method = .int
        return _keyword_count

    get_keyword: method = .keywordclaim
        arg index = .int
        return _keywords[index]

    add_import: method = .void
        arg namespace_name = .string, provenance = .string, flags = .string
        call add_import_plan(.importplan(namespace_name, provenance, flags))

    add_import_plan: method = .void
        arg plan = .importplan
        i = .int
        i = _import_count + 1
        _import_count = i
        _imports[i] = plan

    get_import_count: method = .int
        return _import_count

    get_import: method = .importplan
        arg index = .int
        return _imports[index]

    add_helper: method = .void
        arg helper = .helperplan
        i = .int
        i = _helper_count + 1
        _helper_count = i
        _helpers[i] = helper

    get_helper_count: method = .int
        return _helper_count

    get_helper: method = .helperplan
        arg index = .int
        return _helpers[index]

    add_diagnostic: method = .void
        arg diagnostic = .exitdiagnostic
        i = .int
        i = _diagnostic_count + 1
        _diagnostic_count = i
        _diagnostics[i] = diagnostic

    get_diagnostic_count: method = .int
        return _diagnostic_count

    get_diagnostic: method = .exitdiagnostic
        arg index = .int
        return _diagnostics[index]

    add_note: method = .void
        arg note = .string
        i = .int
        i = _note_count + 1
        _note_count = i
        _notes[i] = note

    get_note_count: method = .int
        return _note_count

    get_note: method = .string
        arg index = .int
        return _notes[index]

    set_error: method = .void
        arg token_index = .int, message = .string, code = ""
        if code = "" then do
            code = message
            message = ""
        end
        _status = "ERROR"
        _diagnostic_count = 0
        call add_diagnostic(.exitdiagnostic("error", token_index, code, message))

exitresult: class
    _status = .string
    _replacement_line_count = .int
    _replacement_lines = .string[]
    _diagnostic_count = .int
    _diagnostics = .exitdiagnostic[]
    _note_count = .int
    _notes = .string[]

    *: factory
        arg status = .string
        if status = "" then status = "EMPTY"
        _status = status
        _replacement_line_count = 0
        _diagnostic_count = 0
        _note_count = 0
        return

    get_status: method = .string
        return _status

    set_status: method = .void
        arg status = .string
        _status = status

    add_replacement_line: method = .void
        arg replacement = .string
        i = .int
        i = _replacement_line_count + 1
        _replacement_line_count = i
        _replacement_lines[i] = replacement

    get_replacement_line_count: method = .int
        return _replacement_line_count

    get_replacement_line: method = .string
        arg index = .int
        return _replacement_lines[index]

    add_diagnostic: method = .void
        arg diagnostic = .exitdiagnostic
        i = .int
        i = _diagnostic_count + 1
        _diagnostic_count = i
        _diagnostics[i] = diagnostic

    get_diagnostic_count: method = .int
        return _diagnostic_count

    get_diagnostic: method = .exitdiagnostic
        arg index = .int
        return _diagnostics[index]

    add_note: method = .void
        arg note = .string
        i = .int
        i = _note_count + 1
        _note_count = i
        _notes[i] = note

    get_note_count: method = .int
        return _note_count

    get_note: method = .string
        arg index = .int
        return _notes[index]

    set_error: method = .void
        arg token_index = .int, message = .string, code = ""
        if code = "" then do
            code = message
            message = ""
        end
        _status = "ERROR"
        _diagnostic_count = 0
        call add_diagnostic(.exitdiagnostic("error", token_index, code, message))
