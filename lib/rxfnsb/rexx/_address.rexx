/* REXX LEVEL B ADDRESS FUNCTIONS */
options levelb
namespace _rxsysb expose _address _new_address_environment _ensure_address_environment _register_address_environment _set_address_environment _current_address_environment _reset_address_environments _enable_native_address_environment _address_execute_system_command _address_unknown_command_response _address_normalize_environment_name _address_normalize_command _address_first_chars _noredir _redir2array _redir2string _array2redir _string2redir addressbinding addressrequest addressresponse addressenvironment systemaddressenvironment pathaddressenvironment nativeaddressenvironment unknownaddressenvironment
import rxfnsb

addressbinding: class
  _kind = .string
  _internal_name = .string
  _external_alias = .string
  _value = .string
  _flags = .string

  *: factory
    arg kind = "var", internal_name = "", external_alias = "", value = "", flags = ""
    _kind = kind
    _internal_name = internal_name
    _external_alias = external_alias
    _value = value
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

  get_flags: method = .string
    return _flags

addressrequest: class
  _environment_name = .string
  _command = .string
  _stdin_endpoint = .binary
  _stdout_endpoint = .binary
  _stderr_endpoint = .binary
  _binding_count = .int
  _bindings = .addressbinding[]
  _flags = .string

  *: factory
    arg environment_name = "", command = "", stdin_endpoint = .binary, stdout_endpoint = .binary, stderr_endpoint = .binary, flags = ""
    _environment_name = environment_name
    _command = command
    _stdin_endpoint = stdin_endpoint
    _stdout_endpoint = stdout_endpoint
    _stderr_endpoint = stderr_endpoint
    _binding_count = 0
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

  add_binding: method = .void
    arg kind = "var", internal_name = .string, external_alias = .string, value = .string, flags = ""
    call add_binding_plan(.addressbinding(kind, internal_name, external_alias, value, flags))

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

addressresponse: class
  _rc = .int
  _condition_name = .string
  _diagnostic_count = .int
  _diagnostics = .string[]

  *: factory
    arg rc = 0, condition_name = ""
    _rc = rc
    _condition_name = condition_name
    _diagnostic_count = 0
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

addressenvironment: interface
  *: factory = .addressenvironment
    arg env_name = .string

  execute: method = .addressresponse
    arg request = .addressrequest

systemaddressenvironment: class implements .addressenvironment
  *: match
    arg env_name = .string
    name = .string
    name = normalize_environment_name(env_name)
    if name = "SYSTEM" | name = "COMMAND" | name = "CMD" | name = "SHELL" then return 100
    return 0

  *: factory
    arg env_name = .string
    return

  execute: method = .addressresponse
    arg request = .addressrequest
    return spawn_request(request)

pathaddressenvironment: class implements .addressenvironment
  *: match
    arg env_name = .string
    if normalize_environment_name(env_name) = "PATH" then return 100
    return 0

  *: factory
    arg env_name = .string
    return

  execute: method = .addressresponse
    arg request = .addressrequest
    return spawn_request(request)

unknownaddressenvironment: class implements .addressenvironment
  _environment_name = .string

  *: match
    arg env_name = .string
    return 1

  *: factory
    arg env_name = .string
    _environment_name = normalize_environment_name(env_name)
    return

  execute: method = .addressresponse
    arg request = .addressrequest
    env_name = .string
    env_name = _environment_name
    if env_name = "" then env_name = request.get_environment_name()
    return unknown_environment_response(env_name)

nativeaddressenvironment: class implements .addressenvironment
  _native_handle = .int

  *: match
    arg env_name = .string
    if native_address_runtime_enabled() = 0 then return 0
    return _native_address_match(env_name)

  *: factory
    arg env_name = .string
    _native_handle = _native_address_handle(env_name)
    return

  execute: method = .addressresponse
    arg request = .addressrequest
    rc = .int
    rc = _native_address_execute(_native_handle, request)
    return .addressresponse(rc)

/* This is the function that the compiler calls for the ADDRESS instruction */
_address: procedure = .int
  arg env = "", command = "", in = .binary, out = .binary, err = .binary, expose ... = .string

  request = .addressrequest
  response = .addressresponse
  env_name = .string
  set_rc = .int

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
    j = .int

    alias = arg.i
    value = ""
    j = i + 1
    if j <= arg.0 then value = arg.j

    call request.add_binding("var", alias, alias, value)
  end

  response = dispatch_address_request(request)
  return response.get_rc()

_new_address_environment: procedure = .addressenvironment
  arg env_name = .string
  return .addressenvironment(normalize_environment_name(env_name))

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

  stdin_endpoint = source_request.get_stdin_endpoint()
  stdout_endpoint = source_request.get_stdout_endpoint()
  stderr_endpoint = source_request.get_stderr_endpoint()
  request = .addressrequest("SYSTEM", command, stdin_endpoint, stdout_endpoint, stderr_endpoint)

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
