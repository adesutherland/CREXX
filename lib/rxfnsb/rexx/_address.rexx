/* REXX LEVEL B ADDRESS FUNCTIONS */
options levelb
namespace _rxsysb expose _address _address_with_sandbox _address_new_request _address_dispatch_request _address_link_request_sandbox _address_apply_response_var _address_apply_response_sandbox _address_apply_response_stem _address_apply_response_request_stem _address_environment _address_function _address_call _address_call_response _new_address_environment _ensure_address_environment _register_address_environment _set_address_environment _current_address_environment _reset_address_environments _enable_native_address_environment _address_execute_system_command _address_unknown_command_response _address_normalize_environment_name _address_normalize_command _address_first_chars _noredir _redir2array _redir2string _array2redir _string2redir addressbinding addressstem standardaddressstem addresssandbox standardaddresssandbox addressrequest addressresponse addressinstance addressfunctionrequest addressfunctionresponse addressfunctionenvironment addressenvironment systemaddressenvironment pathaddressenvironment nativeaddressenvironment unknownaddressenvironment
import rxfnsb

addressstem: interface
  get: method = .string
    arg name = .string

  set: method = .void
    arg name = .string, value = .string

  drop: method = .void
    arg name = .string

  exists: method = .int
    arg name = .string

  next: method = .string
    arg cursor_text = .string

standardaddressstem: class implements .addressstem
  _key_count = .int
  _keys = .string[]
  _values = .string[]
  _present = .int[]

  *: factory
    _key_count = 0
    return

  get: method = .string
    arg name = .string
    idx = .int
    idx = find_sandbox_key_index(_keys, _present, _key_count, name)
    if idx = 0 then return ""
    return _values[idx]

  set: method = .void
    arg name = .string, value = .string
    idx = .int
    key = .string

    key = normalize_sandbox_key(name)
    if key = "" then return

    idx = find_sandbox_key_index(_keys, _present, _key_count, key)
    if idx = 0 then do
      _key_count = _key_count + 1
      idx = _key_count
      _keys[idx] = key
    end

    _values[idx] = value
    _present[idx] = 1
    return

  drop: method = .void
    arg name = .string
    idx = .int
    idx = find_sandbox_key_index(_keys, _present, _key_count, name)
    if idx = 0 then return
    _present[idx] = 0
    _values[idx] = ""
    return

  exists: method = .int
    arg name = .string
    if find_sandbox_key_index(_keys, _present, _key_count, name) = 0 then return 0
    return 1

  next: method = .string
    arg cursor_text = .string
    start = .int
    cursor = .int

    cursor = cursor_text + 0
    start = cursor + 1
    if start < 1 then start = 1

    do i = start to _key_count
      if _present[i] = 1 then return _keys[i]
    end

    return ""

addressbinding: class
  _kind = .string
  _internal_name = .string
  _external_alias = .string
  _value = .string
  _value_object = .addressstem
  _flags = .string

  *: factory
    arg kind = "var", internal_name = "", external_alias = "", value = "", flags = ""
    _kind = kind
    _internal_name = internal_name
    _external_alias = external_alias
    _value = value
    _value_object = .standardaddressstem()
    _flags = flags
    return

  get_kind: method = .string
    return _kind

  get_internal_name: method = .string
    return _internal_name

  get_external_alias: method = .string
    return _external_alias

  get_value: method = .string
    return _value

  get_value_object: method = .addressstem
    return _value_object

  get_stem_value: method = .string
    arg name = .string
    stem = .addressstem
    assembler link stem, _value_object
    return stem.get(name)

  set_stem_value: method = .void
    arg name = .string, value = .string
    stem = .addressstem
    assembler link stem, _value_object
    call stem.set(name, value)
    return

  get_flags: method = .string
    return _flags

addresssandbox: interface
  get: method = .string
    arg name = .string

  set: method = .void
    arg name = .string, value = .string

  drop: method = .void
    arg name = .string

  exists: method = .int
    arg name = .string

  next: method = .string
    arg cursor_text = .string

standardaddresssandbox: class implements .addresssandbox
  _key_count = .int
  _keys = .string[]
  _values = .string[]
  _present = .int[]

  *: factory
    _key_count = 0
    return

  get: method = .string
    arg name = .string
    idx = .int
    idx = find_sandbox_key_index(_keys, _present, _key_count, name)
    if idx = 0 then return ""
    return _values[idx]

  set: method = .void
    arg name = .string, value = .string
    idx = .int
    key = .string

    key = normalize_sandbox_key(name)
    if key = "" then return

    idx = find_sandbox_key_index(_keys, _present, _key_count, key)
    if idx = 0 then do
      _key_count = _key_count + 1
      idx = _key_count
      _keys[idx] = key
    end

    _values[idx] = value
    _present[idx] = 1
    return

  drop: method = .void
    arg name = .string
    idx = .int
    idx = find_sandbox_key_index(_keys, _present, _key_count, name)
    if idx = 0 then return
    _present[idx] = 0
    _values[idx] = ""
    return

  exists: method = .int
    arg name = .string
    if find_sandbox_key_index(_keys, _present, _key_count, name) = 0 then return 0
    return 1

  next: method = .string
    arg cursor_text = .string
    start = .int
    cursor = .int

    cursor = cursor_text + 0
    start = cursor + 1
    if start < 1 then start = 1

    do i = start to _key_count
      if _present[i] = 1 then return _keys[i]
    end

    return ""

addressrequest: class
  _environment_name = .string
  _command = .string
  _stdin_endpoint = .binary
  _stdout_endpoint = .binary
  _stderr_endpoint = .binary
  _binding_count = .int
  _bindings = .addressbinding[]
  _sandbox = .addresssandbox
  _flags = .string

  *: factory
    arg environment_name = "", command = "", stdin_endpoint = .binary, stdout_endpoint = .binary, stderr_endpoint = .binary, flags = ""
    _environment_name = environment_name
    _command = command
    _stdin_endpoint = stdin_endpoint
    _stdout_endpoint = stdout_endpoint
    _stderr_endpoint = stderr_endpoint
    _binding_count = 0
    _sandbox = .standardaddresssandbox()
    _flags = flags
    return

  get_environment_name: method = .string
    return _environment_name

  set_environment_name: method = .void
    arg environment_name = .string
    _environment_name = environment_name

  get_command: method = .string
    return _command

  set_command: method = .void
    arg command = .string
    _command = command

  get_stdin_endpoint: method = .binary
    return _stdin_endpoint

  set_stdin_endpoint: method = .void
    arg stdin_endpoint = .binary
    _stdin_endpoint = stdin_endpoint

  get_stdout_endpoint: method = .binary
    return _stdout_endpoint

  set_stdout_endpoint: method = .void
    arg stdout_endpoint = .binary
    _stdout_endpoint = stdout_endpoint

  get_stderr_endpoint: method = .binary
    return _stderr_endpoint

  set_stderr_endpoint: method = .void
    arg stderr_endpoint = .binary
    _stderr_endpoint = stderr_endpoint

  get_flags: method = .string
    return _flags

  get_sandbox: method = .addresssandbox
    return _sandbox

  set_sandbox: method = .void
    arg sandbox = .addresssandbox
    _sandbox = sandbox

  get_sandbox_value: method = .string
    arg name = .string
    sandbox = .addresssandbox
    assembler link sandbox, _sandbox
    return sandbox.get(name)

  set_sandbox_value: method = .void
    arg name = .string, value = .string
    sandbox = .addresssandbox
    assembler link sandbox, _sandbox
    call sandbox.set(name, value)
    return

  add_binding: method = .void
    arg kind = "var", internal_name = .string, external_alias = .string, value = .string, flags = ""
    call add_binding_plan(.addressbinding(kind, internal_name, external_alias, value, flags))

  add_stem_binding_plan: method = .void
    arg internal_name = .string, external_alias = .string, stem = .addressstem
    binding = .addressbinding
    binding = .addressbinding("stem", internal_name, external_alias, "", "")
    assembler linktoattr1 6, binding, stem
    call add_binding_plan(binding)

  add_binding_plan: method = .void
    arg binding = .addressbinding
    i = .int
    i = _binding_count + 1
    _binding_count = i
    _bindings[i] = binding

  get_binding_count: method = .int
    return _binding_count

  get_binding: method = .addressbinding
    arg index = .int
    return _bindings[index]

  get_binding_value: method = .string
    arg name = .string
    binding = .addressbinding
    target = .string

    target = normalize_environment_name(name)
    if target = "" then return ""

    do i = 1 to _binding_count
      binding = _bindings[i]
      if normalize_environment_name(binding.get_kind()) \= "VAR" then iterate
      if normalize_environment_name(binding.get_internal_name()) = target then return binding.get_value()
      if normalize_environment_name(binding.get_external_alias()) = target then return binding.get_value()
    end

    return ""

  get_binding_stem_value: method = .string
    arg index = .int, name = .string
    binding = .addressbinding
    assembler linkattr1 binding, _bindings, index
    return binding.get_stem_value(name)

  set_binding_stem_value: method = .void
    arg index = .int, name = .string, value = .string
    binding = .addressbinding
    assembler linkattr1 binding, _bindings, index
    call binding.set_stem_value(name, value)
    return

addressresponse: class
  _rc = .int
  _condition_name = .string
  _diagnostic_count = .int
  _diagnostics = .string[]
  _updated_binding_count = .int
  _updated_bindings = .addressbinding[]

  *: factory
    arg rc = 0, condition_name = ""
    _rc = rc
    _condition_name = condition_name
    _diagnostic_count = 0
    _updated_binding_count = 0
    return

  get_rc: method = .int
    return _rc

  set_rc: method = .void
    arg rc = .int
    _rc = rc

  get_condition_name: method = .string
    return _condition_name

  set_condition_name: method = .void
    arg condition_name = .string
    _condition_name = condition_name

  add_diagnostic: method = .void
    arg text = .string
    i = .int
    i = _diagnostic_count + 1
    _diagnostic_count = i
    _diagnostics[i] = text

  get_diagnostic_count: method = .int
    return _diagnostic_count

  get_diagnostic: method = .string
    arg index = .int
    return _diagnostics[index]

  add_updated_binding: method = .void
    arg kind = "var", internal_name = .string, external_alias = .string, value = .string, flags = ""
    call add_updated_binding_plan(.addressbinding(kind, internal_name, external_alias, value, flags))

  add_updated_binding_plan: method = .void
    arg binding = .addressbinding
    i = .int
    i = _updated_binding_count + 1
    _updated_binding_count = i
    _updated_bindings[i] = binding

  get_updated_binding_count: method = .int
    return _updated_binding_count

  get_updated_binding: method = .addressbinding
    arg index = .int
    return _updated_bindings[index]

addressinstance: interface
  bind_environment: method = .void
    arg env_name = .string, instance_id = .string

  environment_name: method = .string

  environment_id: method = .string

addressfunctionrequest: class
  _arguments = .string[]
  _environment_name = .string
  _flags = .string
  _function_name = .string
  _sandbox = .addresssandbox

  *: factory
    arg environment_name = .string, function_name = .string, arguments = .string[], sandbox = .addresssandbox, flags = ""
    _environment_name = environment_name
    _function_name = function_name
    _arguments = arguments
    _sandbox = sandbox
    _flags = flags
    return

  get_environment_name: method = .string
    return _environment_name

  get_function_name: method = .string
    return _function_name

  get_argument_count: method = .int
    return _arguments.0

  get_argument: method = .string
    arg index = .int
    if index < 1 | index > _arguments.0 then return ""
    return _arguments[index]

  get_sandbox: method = .addresssandbox
    return _sandbox

  get_flags: method = .string
    return _flags

addressfunctionresponse: class
  _condition_name = .string
  _diagnostic_count = .int
  _diagnostics = .string[]
  _rc = .int
  _result = .string

  *: factory
    arg rc = 0, result = "", condition_name = ""
    _rc = rc
    _result = result
    _condition_name = condition_name
    _diagnostic_count = 0
    return

  get_rc: method = .int
    return _rc

  set_rc: method = .void
    arg rc = .int
    _rc = rc

  get_result: method = .string
    return _result

  set_result: method = .void
    arg result = .string
    _result = result

  get_condition_name: method = .string
    return _condition_name

  set_condition_name: method = .void
    arg condition_name = .string
    _condition_name = condition_name

  add_diagnostic: method = .void
    arg text = .string
    i = .int
    i = _diagnostic_count + 1
    _diagnostic_count = i
    _diagnostics[i] = text

  get_diagnostic_count: method = .int
    return _diagnostic_count

  get_diagnostic: method = .string
    arg index = .int
    return _diagnostics[index]

addressfunctionenvironment: interface
  invoke: method = .addressfunctionresponse
    arg request = .addressfunctionrequest

addressenvironment: interface
  *: factory
    arg env_name = .string

  execute: method = .addressresponse
    arg request = .addressrequest

systemaddressenvironment: class implements .addressenvironment .addressinstance .addressfunctionenvironment
  _environment_id = .string
  _environment_name = .string

  *: match
    arg env_name = .string
    name = .string
    name = normalize_environment_name(env_name)
    if name = "SYSTEM" | name = "COMMAND" | name = "CMD" | name = "SHELL" then return 100
    return 0

  *: factory
    arg env_name = .string
    _environment_name = normalize_environment_name(env_name)
    _environment_id = _environment_name
    return

  bind_environment: method = .void
    arg env_name = .string, instance_id = .string
    _environment_name = normalize_environment_name(env_name)
    _environment_id = instance_id
    if _environment_id = "" then _environment_id = _environment_name
    return

  environment_name: method = .string
    return _environment_name

  environment_id: method = .string
    return _environment_id

  execute: method = .addressresponse
    arg request = .addressrequest
    return spawn_request(request)

  invoke: method = .addressfunctionresponse
    arg request = .addressfunctionrequest
    return unsupported_function_response(_environment_name, request.get_function_name())

pathaddressenvironment: class implements .addressenvironment .addressinstance .addressfunctionenvironment
  _environment_id = .string
  _environment_name = .string

  *: match
    arg env_name = .string
    if normalize_environment_name(env_name) = "PATH" then return 100
    return 0

  *: factory
    arg env_name = .string
    _environment_name = normalize_environment_name(env_name)
    _environment_id = _environment_name
    return

  bind_environment: method = .void
    arg env_name = .string, instance_id = .string
    _environment_name = normalize_environment_name(env_name)
    _environment_id = instance_id
    if _environment_id = "" then _environment_id = _environment_name
    return

  environment_name: method = .string
    return _environment_name

  environment_id: method = .string
    return _environment_id

  execute: method = .addressresponse
    arg request = .addressrequest
    return spawn_request(request)

  invoke: method = .addressfunctionresponse
    arg request = .addressfunctionrequest
    return unsupported_function_response(_environment_name, request.get_function_name())

unknownaddressenvironment: class implements .addressenvironment .addressinstance .addressfunctionenvironment
  _environment_id = .string
  _environment_name = .string

  *: match
    arg env_name = .string
    return 1

  *: factory
    arg env_name = .string
    _environment_name = normalize_environment_name(env_name)
    _environment_id = _environment_name
    return

  bind_environment: method = .void
    arg env_name = .string, instance_id = .string
    _environment_name = normalize_environment_name(env_name)
    _environment_id = instance_id
    if _environment_id = "" then _environment_id = _environment_name
    return

  environment_name: method = .string
    return _environment_name

  environment_id: method = .string
    return _environment_id

  execute: method = .addressresponse
    arg request = .addressrequest
    env_name = .string
    env_name = _environment_name
    if env_name = "" then env_name = request.get_environment_name()
    return unknown_environment_response(env_name)

  invoke: method = .addressfunctionresponse
    arg request = .addressfunctionrequest
    return unsupported_function_response(_environment_name, request.get_function_name())

nativeaddressenvironment: class implements .addressenvironment .addressinstance .addressfunctionenvironment
  _environment_id = .string
  _environment_name = .string
  _native_handle = .int

  *: match
    arg env_name = .string
    if native_address_runtime_enabled() = 0 then return 0
    return _native_address_match(env_name)

  *: factory
    arg env_name = .string
    _environment_name = normalize_environment_name(env_name)
    _native_handle = _native_address_handle(env_name)
    _environment_id = _native_address_id(env_name)
    if _environment_id = "" then _environment_id = _environment_name
    return

  bind_environment: method = .void
    arg env_name = .string, instance_id = .string
    _environment_name = normalize_environment_name(env_name)
    _environment_id = instance_id
    if _environment_id = "" then _environment_id = _native_address_id(_environment_name)
    if _environment_id = "" then _environment_id = _environment_name
    return

  environment_name: method = .string
    return _environment_name

  environment_id: method = .string
    return _environment_id

  execute: method = .addressresponse
    arg request = .addressrequest
    rc = .int
    response = .addressresponse
    response = .addressresponse(0)
    rc = _native_address_execute(_native_handle, request, response)
    call response.set_rc(rc)
    return response

  invoke: method = .addressfunctionresponse
    arg request = .addressfunctionrequest
    rc = .int
    response = .addressfunctionresponse
    response = .addressfunctionresponse(0, "")
    rc = _native_address_invoke(_native_handle, request, response)
    call response.set_rc(rc)
    return response

/* This is the function that the compiler calls for the ADDRESS instruction */
_address: procedure = .int
  arg env = "", command = "", in = .binary, out = .binary, err = .binary, expose ... = .string

  request = .addressrequest
  response = .addressresponse
  env_name = .string
  set_rc = .int
  update = .addressbinding
  update_kind = .string
  update_internal = .string
  update_alias = .string
  exposed_name = .string
  update_value = .string
  j = .int

  call ensure_address_runtime

  env_name = normalize_environment_name(env)
  if env_name \= "" then do
    set_rc = _set_address_environment(env_name)
    if set_rc \= 0 then return set_rc
    env_name = _current_address_environment()
  end
  else env_name = _current_address_environment()

  request = .addressrequest(env_name, command, in, out, err)

  do i = 1 to arg.0 by 2
    alias = .string
    value = .string

    alias = arg.i
    value = ""
    j = i + 1
    if j <= arg.0 then value = arg.j

    call request.add_binding("var", alias, alias, value)
  end

  response = dispatch_address_request(request)

  do update_index = 1 to response.get_updated_binding_count()
    update = response.get_updated_binding(update_index)
    update_kind = normalize_environment_name(update.get_kind())
    if update_kind = "SANDBOX" then do
      update_internal = update.get_internal_name()
      update_alias = update.get_external_alias()
      if update_internal = "" then update_internal = update_alias
      if update_internal = "" then iterate
      update_value = update.get_value()
      call request.set_sandbox_value(update_internal, update_value)
      iterate
    end
    if update_kind \= "VAR" then iterate

    update_internal = update.get_internal_name()
    update_alias = update.get_external_alias()
    if update_alias = "" then update_alias = update_internal
    update_value = update.get_value()

    do i = 1 to arg.0 by 2
      j = i + 1
      if j > arg.0 then iterate

      exposed_name = arg.i
      if normalize_environment_name(exposed_name) = normalize_environment_name(update_internal) | normalize_environment_name(exposed_name) = normalize_environment_name(update_alias) then do
        call _address_apply_exposed_value(arg[j], update_value)
      end
    end
  end
  return response.get_rc()

/* ADDRESS dispatch with an explicit caller-supplied sandbox object. */
_address_with_sandbox: procedure = .int
  arg env = "", command = "", in = .binary, out = .binary, err = .binary, expose sandbox = .addresssandbox, expose ... = .string

  request = .addressrequest
  response = .addressresponse
  env_name = .string
  set_rc = .int
  update = .addressbinding
  update_kind = .string
  update_internal = .string
  update_alias = .string
  exposed_name = .string
  update_value = .string
  j = .int

  call ensure_address_runtime

  env_name = normalize_environment_name(env)
  if env_name \= "" then do
    set_rc = _set_address_environment(env_name)
    if set_rc \= 0 then return set_rc
    env_name = _current_address_environment()
  end
  else env_name = _current_address_environment()

  request = .addressrequest(env_name, command, in, out, err)
  /* Keep C and Rexx providers on the same caller-supplied sandbox object. */
  assembler linktoattr1 6, request, sandbox

  do i = 1 to arg.0 by 2
    alias = .string
    value = .string

    alias = arg.i
    value = ""
    j = i + 1
    if j <= arg.0 then value = arg.j

    call request.add_binding("var", alias, alias, value)
  end

  response = dispatch_address_request(request)

  do update_index = 1 to response.get_updated_binding_count()
    update = response.get_updated_binding(update_index)
    update_kind = normalize_environment_name(update.get_kind())
    if update_kind = "SANDBOX" then do
      update_internal = update.get_internal_name()
      update_alias = update.get_external_alias()
      if update_internal = "" then update_internal = update_alias
      if update_internal = "" then iterate
      update_value = update.get_value()
      call sandbox.set(update_internal, update_value)
      iterate
    end
    if update_kind \= "VAR" then iterate

    update_internal = update.get_internal_name()
    update_alias = update.get_external_alias()
    if update_alias = "" then update_alias = update_internal
    update_value = update.get_value()

    do i = 1 to arg.0 by 2
      j = i + 1
      if j > arg.0 then iterate

      exposed_name = arg.i
      if normalize_environment_name(exposed_name) = normalize_environment_name(update_internal) | normalize_environment_name(exposed_name) = normalize_environment_name(update_alias) then do
        call _address_apply_exposed_value(arg[j], update_value)
      end
    end
  end
  return response.get_rc()

_address_apply_exposed_value: procedure = .void
  arg expose target = .string, value = .string
  target = value
  return

_address_new_request: procedure = .addressrequest
  arg env = "", command = "", in = .binary, out = .binary, err = .binary

  env_name = .string
  set_rc = .int

  call ensure_address_runtime

  env_name = normalize_environment_name(env)
  if env_name \= "" then do
    set_rc = _set_address_environment(env_name)
    if set_rc \= 0 then return .addressrequest("", command, in, out, err)
    env_name = _current_address_environment()
  end
  else env_name = _current_address_environment()

  return .addressrequest(env_name, command, in, out, err)

_address_dispatch_request: procedure = .addressresponse
  arg request = .addressrequest
  return dispatch_address_request(request)

_address_link_request_sandbox: procedure = .void
  arg request = .addressrequest, expose sandbox = .addresssandbox
  assembler linktoattr1 6, request, sandbox
  return

_address_apply_response_var: procedure = .void
  arg response = .addressresponse, internal_name = .string, expose target = .string

  update = .addressbinding
  update_kind = .string
  update_internal = .string
  update_alias = .string

  do update_index = 1 to response.get_updated_binding_count()
    update = response.get_updated_binding(update_index)
    update_kind = normalize_environment_name(update.get_kind())
    if update_kind \= "VAR" then iterate

    update_internal = update.get_internal_name()
    update_alias = update.get_external_alias()
    if update_alias = "" then update_alias = update_internal

    if normalize_environment_name(internal_name) = normalize_environment_name(update_internal) | normalize_environment_name(internal_name) = normalize_environment_name(update_alias) then do
      target = update.get_value()
    end
  end
  return

_address_apply_response_sandbox: procedure = .void
  arg response = .addressresponse, expose sandbox = .addresssandbox

  update = .addressbinding
  update_kind = .string
  update_internal = .string
  update_alias = .string
  update_value = .string

  do update_index = 1 to response.get_updated_binding_count()
    update = response.get_updated_binding(update_index)
    update_kind = normalize_environment_name(update.get_kind())
    if update_kind \= "SANDBOX" then iterate

    update_internal = update.get_internal_name()
    update_alias = update.get_external_alias()
    if update_internal = "" then update_internal = update_alias
    if update_internal = "" then iterate
    update_value = update.get_value()
    call sandbox.set(update_internal, update_value)
  end
  return

_address_apply_response_stem: procedure = .void
  arg response = .addressresponse, internal_name = .string, stem = .addressstem

  update = .addressbinding
  update_kind = .string
  update_internal = .string
  update_alias = .string
  update_value = .string
  stem_prefix = .string
  update_key = .string

  stem_prefix = normalize_environment_name(internal_name) || "."

  do update_index = 1 to response.get_updated_binding_count()
    update = response.get_updated_binding(update_index)
    update_kind = normalize_environment_name(update.get_kind())
    if update_kind \= "STEM" then iterate

    update_internal = update.get_internal_name()
    update_alias = update.get_external_alias()
    if update_internal = "" then update_internal = update_alias
    if update_internal = "" then iterate

    update_key = ""
    if left(normalize_environment_name(update_internal), length(stem_prefix)) = stem_prefix then do
      update_key = substr(update_internal, length(stem_prefix) + 1)
    end
    else if normalize_environment_name(update_alias) = normalize_environment_name(internal_name) then update_key = update_internal
    else iterate

    update_value = update.get_value()
    call stem.set(update_key, update_value)
  end
  return

_address_apply_response_request_stem: procedure = .void
  arg response = .addressresponse, request = .addressrequest, binding_index = .int, internal_name = .string

  update = .addressbinding
  update_kind = .string
  update_internal = .string
  update_alias = .string
  update_value = .string
  stem_prefix = .string
  update_key = .string

  stem_prefix = normalize_environment_name(internal_name) || "."

  do update_index = 1 to response.get_updated_binding_count()
    update = response.get_updated_binding(update_index)
    update_kind = normalize_environment_name(update.get_kind())
    if update_kind \= "STEM" then iterate

    update_internal = update.get_internal_name()
    update_alias = update.get_external_alias()
    if update_internal = "" then update_internal = update_alias
    if update_internal = "" then iterate

    update_key = ""
    if left(normalize_environment_name(update_internal), length(stem_prefix)) = stem_prefix then do
      update_key = substr(update_internal, length(stem_prefix) + 1)
    end
    else if normalize_environment_name(update_alias) = normalize_environment_name(internal_name) then update_key = update_internal
    else iterate

    update_value = update.get_value()
    call request.set_binding_stem_value(binding_index, update_key, update_value)
  end
  return

_address_environment: procedure = .addressenvironment expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  arg env_name = ""

  name = .string
  idx = .int
  ensure_rc = .int

  call ensure_address_runtime

  name = normalize_environment_name(env_name)
  if name = "" then name = _current_address_environment()

  ensure_rc = _ensure_address_environment(name)
  if ensure_rc \= 0 then return .unknownaddressenvironment(name)

  idx = find_address_environment_index(_address_environment_names, name)
  if idx = 0 then return .unknownaddressenvironment(name)
  return _address_environment_objects[idx] as .addressenvironment

_address_function: procedure = .addressfunctionresponse
  arg env_name = "", function_name = "", arguments = .string[]

  env_obj = .addressenvironment
  fn_env = .addressfunctionenvironment
  request = .addressfunctionrequest
  request_env = .string
  sandbox = .addresssandbox

  request_env = normalize_environment_name(env_name)
  if request_env = "" then request_env = _current_address_environment()
  env_obj = _address_environment(env_name)
  if env_obj is .addressfunctionenvironment then do
    fn_env = env_obj as .addressfunctionenvironment
    sandbox = .standardaddresssandbox()
    request = .addressfunctionrequest(request_env, function_name, arguments, sandbox, "")
    return fn_env.invoke(request)
  end

  return unsupported_function_response(request_env, function_name)

_address_call_response: procedure = .addressfunctionresponse
  arg env_name = "", function_name = "", ... = .string

  arguments = .string[]
  do i = 1 to arg.0
    arguments[i] = arg.i
  end

  return _address_function(env_name, function_name, arguments)

_address_call: procedure = .string
  arg env_name = "", function_name = "", ... = .string

  arguments = .string[]
  response = .addressfunctionresponse

  do i = 1 to arg.0
    arguments[i] = arg.i
  end

  response = _address_function(env_name, function_name, arguments)
  return response.get_result()

_new_address_environment: procedure = .addressenvironment
  arg env_name = .string
  return .addressenvironment(normalize_environment_name(env_name))

bind_address_environment_object: procedure = .void
  arg env_name = .string, env_obj = .addressenvironment, instance_id = ""

  instance = .addressinstance
  bound_id = .string

  if env_obj is .addressinstance then do
    instance = env_obj as .addressinstance
    bound_id = instance_id
    if bound_id = "" then bound_id = instance.environment_id()
    call instance.bind_environment(env_name, bound_id)
  end
  return

_ensure_address_environment: procedure = .int expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  arg env_name = .string

  name = .string
  idx = .int
  env_obj = .addressenvironment

  call ensure_address_runtime

  name = normalize_environment_name(env_name)
  if name = "" then return -3

  idx = find_address_environment_index(_address_environment_names, name)
  if idx \= 0 then return 0

  env_obj = .addressenvironment(name)
  call bind_address_environment_object(name, env_obj, "")
  idx = _address_environment_names.0 + 1
  _address_environment_names.idx = name
  _address_environment_objects.idx = env_obj
  return 0

_register_address_environment: procedure = .int expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  arg env_name = .string, env_obj = .addressenvironment

  name = .string
  idx = .int

  call ensure_address_runtime

  name = normalize_environment_name(env_name)
  if name = "" then return -3

  idx = find_address_environment_index(_address_environment_names, name)
  if idx = 0 then idx = _address_environment_names.0 + 1

  call bind_address_environment_object(name, env_obj, "")
  _address_environment_names.idx = name
  _address_environment_objects.idx = env_obj
  return 0

_set_address_environment: procedure = .int expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  arg env_name = .string

  name = .string
  idx = .int
  ensure_rc = .int

  call ensure_address_runtime

  name = normalize_environment_name(env_name)
  if name = "" then return -3

  ensure_rc = _ensure_address_environment(name)
  if ensure_rc \= 0 then return ensure_rc

  idx = find_address_environment_index(_address_environment_names, name)
  if idx = 0 then return -3

  _address_current_name = name
  return 0

_current_address_environment: procedure = .string expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  call ensure_address_runtime
  if _address_current_name = "" then _address_current_name = "SYSTEM"
  return _address_current_name

_reset_address_environments: procedure = .int expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  _address_runtime_ready = 0
  _address_current_name = ""
  _address_environment_names = .string[]
  _address_environment_objects = .addressenvironment[]
  call ensure_address_runtime
  return 0

_enable_native_address_environment: procedure = .int expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  idx = .int

  call ensure_address_runtime
  idx = find_address_environment_index(_address_environment_names, "__NATIVE_ADDRESS_PROVIDER__")
  if idx = 0 then do
    idx = _address_environment_names.0 + 1
    _address_environment_names.idx = "__NATIVE_ADDRESS_PROVIDER__"
    _address_environment_objects.idx = .unknownaddressenvironment("__NATIVE_ADDRESS_PROVIDER__")
  end

  return 0

native_address_runtime_enabled: procedure = .int expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  call ensure_address_runtime
  if find_address_environment_index(_address_environment_names, "__NATIVE_ADDRESS_PROVIDER__") \= 0 then return 1
  return 0

ensure_address_runtime: procedure expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  if _address_runtime_ready = 1 then return

  _address_environment_names = .string[]
  _address_environment_objects = .addressenvironment[]
  _address_current_name = "SYSTEM"
  _address_runtime_ready = 1

  call _ensure_address_environment("SYSTEM")
  call alias_address_environment("COMMAND", "SYSTEM")
  call alias_address_environment("CMD", "SYSTEM")
  call alias_address_environment("SHELL", "SYSTEM")
  call _ensure_address_environment("PATH")

  return

alias_address_environment: procedure = .int expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  arg alias_name = .string, target_name = .string

  alias = .string
  target = .string
  alias_idx = .int
  target_idx = .int
  ensure_rc = .int

  alias = normalize_environment_name(alias_name)
  target = normalize_environment_name(target_name)
  if alias = "" | target = "" then return -3

  ensure_rc = _ensure_address_environment(target)
  if ensure_rc \= 0 then return ensure_rc

  target_idx = find_address_environment_index(_address_environment_names, target)
  if target_idx = 0 then return -3

  alias_idx = find_address_environment_index(_address_environment_names, alias)
  if alias_idx = 0 then alias_idx = _address_environment_names.0 + 1

  _address_environment_names.alias_idx = alias
  _address_environment_objects.alias_idx = _address_environment_objects[target_idx] as .addressenvironment
  return 0

find_address_environment_index: procedure = .int
  arg expose env_names = .string[], env_name = .string

  name = .string
  name = normalize_environment_name(env_name)

  do i = 1 to env_names.0
    if normalize_environment_name(env_names.i) = name then return i
  end

  return 0

normalize_sandbox_key: procedure = .string
  arg name = .string
  normalized = .string
  normalized = strip(name)
  normalized = upper(normalized)
  return normalized

find_sandbox_key_index: procedure = .int
  arg expose keys = .string[], present = .int[], key_count = .int, name = .string

  key = .string
  key = normalize_sandbox_key(name)
  if key = "" then return 0

  do i = 1 to key_count
    if present[i] = 1 & keys[i] = key then return i
  end

  return 0

dispatch_address_request: procedure = .addressresponse expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  arg request = .addressrequest

  env_name = .string
  idx = .int
  env_obj = .addressenvironment

  call ensure_address_runtime

  env_name = normalize_environment_name(request.get_environment_name())
  if env_name = "" then env_name = _current_address_environment()

  idx = find_address_environment_index(_address_environment_names, env_name)
  if idx = 0 then do
    call _ensure_address_environment(env_name)
    idx = find_address_environment_index(_address_environment_names, env_name)
  end
  if idx = 0 then return unknown_environment_response(env_name)

  env_obj = _address_environment_objects[idx] as .addressenvironment
  return env_obj.execute(request)

execute_system_command: procedure = .addressresponse expose _address_runtime_ready _address_current_name _address_environment_names _address_environment_objects
  arg source_request = .addressrequest, command = .string

  request = .addressrequest
  env_obj = .addressenvironment
  idx = .int
  stdin_endpoint = .binary
  stdout_endpoint = .binary
  stderr_endpoint = .binary
  sandbox = .addresssandbox

  stdin_endpoint = source_request.get_stdin_endpoint()
  stdout_endpoint = source_request.get_stdout_endpoint()
  stderr_endpoint = source_request.get_stderr_endpoint()
  request = .addressrequest("SYSTEM", command, stdin_endpoint, stdout_endpoint, stderr_endpoint)
  assembler linkattr1 sandbox, source_request, 6
  assembler linktoattr1 6, request, sandbox

  do i = 1 to source_request.get_binding_count()
    call request.add_binding_plan(source_request.get_binding(i))
  end

  call ensure_address_runtime
  call _ensure_address_environment("SYSTEM")
  idx = find_address_environment_index(_address_environment_names, "SYSTEM")
  if idx = 0 then return unknown_environment_response("SYSTEM")

  env_obj = _address_environment_objects[idx] as .addressenvironment
  return env_obj.execute(request)

_address_execute_system_command: procedure = .addressresponse
  arg source_request = .addressrequest, command = .string
  return execute_system_command(source_request, command)

_address_unknown_command_response: procedure = .addressresponse
  arg env_name = .string, command = .string
  return unknown_command_response(env_name, command)

_address_normalize_environment_name: procedure = .string
  arg env_name = .string
  return normalize_environment_name(env_name)

_address_normalize_command: procedure = .string
  arg command = .string
  return normalize_command(command)

_address_first_chars: procedure = .string
  arg text = .string, count = .int
  return first_chars(text, count)

spawn_request: procedure = .addressresponse
  arg request = .addressrequest

  response = .addressresponse(0)
  redirect = .binary[4]
  envs = .string[]
  slot = .int
  command = .string
  stdin_endpoint = .binary
  stdout_endpoint = .binary
  stderr_endpoint = .binary

  stdin_endpoint = request.get_stdin_endpoint()
  stdout_endpoint = request.get_stdout_endpoint()
  stderr_endpoint = request.get_stderr_endpoint()

  redirect[1] = stdin_endpoint
  redirect[2] = stdout_endpoint
  redirect[3] = stderr_endpoint

  assembler linkattr1 envs,redirect,4

  slot = 0
  do i = 1 to request.get_binding_count()
    binding = .addressbinding
    alias = .string
    kind = .string

    binding = request.get_binding(i)
    kind = normalize_environment_name(binding.get_kind())
    if kind \= "VAR" then iterate

    alias = binding.get_external_alias()
    if alias = "" then alias = binding.get_internal_name()

    slot = slot + 1
    envs.slot = alias
    slot = slot + 1
    envs.slot = binding.get_value()
  end

  assembler unlink envs

  command = request.get_command()
  rc = 0
  assembler spawn rc,command,redirect

  call response.set_rc(rc)
  return response

unknown_environment_response: procedure = .addressresponse
  arg env_name = .string

  response = .addressresponse(0)
  call response.set_rc(-3)
  call response.set_condition_name("FAILURE")
  call response.add_diagnostic("Unknown address environment " || env_name)
  return response

unknown_command_response: procedure = .addressresponse
  arg env_name = .string, command = .string

  response = .addressresponse(0)
  call response.set_rc(-3)
  call response.set_condition_name("FAILURE")
  call response.add_diagnostic("Unknown command for " || env_name || ": " || command)
  return response

unsupported_function_response: procedure = .addressfunctionresponse
  arg env_name = .string, function_name = .string

  response = .addressfunctionresponse(-3, "")
  call response.set_condition_name("FAILURE")
  call response.add_diagnostic("Unknown address function for " || env_name || ": " || function_name)
  return response

normalize_environment_name: procedure = .string
  arg env_name = .string
  normalized = .string
  normalized = strip(env_name)
  normalized = upper(normalized)
  return normalized

normalize_command: procedure = .string
  arg command = .string
  normalized = .string
  normalized = strip(command)
  normalized = space(normalized)
  normalized = upper(normalized)
  return normalized

first_chars: procedure = .string
  arg text = .string, count = .int
  if count <= 0 then return ""
  return substr(text, 1, count)

/* Return a no redirect dummy Native Object */
_noredir: procedure = .binary
    handle = .binary
    return handle

/* Return a redirect to Array Native Object */
_redir2array: procedure = .binary
    arg expose arr = .string[]
    handle = .binary
    assembler redir2arr handle, arr
    return handle

/* Return a redirect to String Native Object */
_redir2string: procedure = .binary
    arg expose str = .string
    handle = .binary
    assembler redir2str handle, str
    return handle

/* Return a redirect from an Array Native Object */
_array2redir: procedure = .binary
    arg expose arr = .string[]
    handle = .binary
    assembler arr2redir handle, arr
    return handle

/* Return a redirect from a String Native Object */
_string2redir: procedure = .binary
    arg expose str = .string
    handle = .binary
    assembler str2redir handle, str
    return handle
