/* REXX Level B Signal Objects */
options levelb

namespace rxfnsb expose signal standard_signal runtime_signal signalaction standard_signalaction

signal: interface
  *: factory
    arg name = .string, message = ""

  name: method = .string
  code: method = .int
  module: method = .int
  address: method = .int
  message: method = .string
  payload: method = .object
  file: method = .string
  line: method = .int
  column: method = .int
  source: method = .string

standard_signal: class implements .signal
  _name = .string
  _message = .string
  _payload = .object

  *: factory
    arg name = .string, message = ""
    _name = _signal_name(name)
    _message = message
    return

  name: method = .string
    return _name

  code: method = .int
    return _signal_code(_name)

  module: method = .int
    return 0

  address: method = .int
    return 0

  message: method = .string
    return _message

  payload: method = .object
    return _payload

  file: method = .string
    return ""

  line: method = .int
    return 0

  column: method = .int
    return 0

  source: method = .string
    return ""

runtime_signal: class implements .signal
  _raw = .object
  _name = .string
  _message = .string
  _payload = .object
  _has_raw = .int

  *: factory
    arg name = .string, message = ""
    _name = _signal_name(name)
    _message = message
    _has_raw = 0
    return

  set_raw: method = .void
    arg raw = .object
    _raw = raw
    _has_raw = 1
    return

  name: method = .string
    result = .string
    if _has_raw = 0 then return _name
    assembler linkattr1 result, _raw, 4
    return result

  code: method = .int
    result = .int
    if _has_raw = 0 then return _signal_code(_name)
    assembler linkattr1 result, _raw, 1
    return result

  module: method = .int
    result = .int
    if _has_raw = 0 then return 0
    assembler linkattr1 result, _raw, 2
    return result

  address: method = .int
    result = .int
    if _has_raw = 0 then return 0
    assembler linkattr1 result, _raw, 3
    return result

  message: method = .string
    result = .string
    if _has_raw = 0 then return _message
    assembler linkattr1 result, _raw, 5
    return result

  payload: method = .object
    result = .object
    if _has_raw = 0 then return _payload
    assembler linkattr1 result, _raw, 5
    return result

  file: method = .string
    return _signal_metadata_string(module(), address(), ".meta_file", 1)

  line: method = .int
    return _signal_metadata_int(module(), address(), ".meta_src", 1)

  column: method = .int
    return _signal_metadata_int(module(), address(), ".meta_src", 2)

  source: method = .string
    return _signal_metadata_string(module(), address(), ".meta_src", 3)

signalaction: interface
  retry: factory
  skip: factory
  fail: factory
  kind: method = .string

standard_signalaction: class implements .signalaction
  _kind = .string

  retry: factory
    _kind = "retry"
    return

  skip: factory
    _kind = "skip"
    return

  fail: factory
    _kind = "fail"
    return

  kind: method = .string
    return _kind

_signal_name: procedure = .string
  arg name = .string
  return upper(name)

_signal_code: procedure = .int
  arg name = .string
  n = .string
  n = upper(name)
  if n = "KILL" then return 1
  if n = "FAILURE" then return 2
  if n = "ERROR" | n = "SYNTAX" then return 3
  if n = "OVERFLOW_UNDERFLOW" then return 4
  if n = "DIVISION_BY_ZERO" then return 5
  if n = "CONVERSION_ERROR" then return 6
  if n = "INVALID_ARGUMENTS" then return 7
  if n = "OUT_OF_RANGE" then return 8
  if n = "UNICODE_ERROR" then return 9
  if n = "UNKNOWN_INSTRUCTION" then return 10
  if n = "FUNCTION_NOT_FOUND" then return 11
  if n = "NOT_IMPLEMENTED" then return 12
  if n = "INVALID_SIGNAL_CODE" then return 13
  if n = "NOTREADY" then return 15
  if n = "QUIT" then return 19
  if n = "TERM" then return 20
  if n = "POSIX_INT" then return 21
  if n = "POSIX_HUP" then return 22
  if n = "POSIX_USR1" then return 23
  if n = "POSIX_USR2" then return 24
  if n = "POSIX_CHLD" then return 25
  if n = "OTHER" then return 30
  if n = "BREAKPOINT" then return 31
  return 13

_signal_metadata_string: procedure = .string
  arg module = .int, address = .int, wanted = .string, attr = .int
  meta_array = 0
  meta_entry = ""
  result = ""

  if module <= 0 then return ""
  if address < 0 then return ""

  do a = address to 0 by -1
    assembler metaloaddata meta_array, module, a
    do i = 1 to meta_array
      assembler linkattr1 meta_entry, meta_array, i
      if meta_entry = wanted then do
        assembler linkattr1 result, meta_entry, attr
        return result
      end
    end
  end

  return ""

_signal_metadata_int: procedure = .int
  arg module = .int, address = .int, wanted = .string, attr = .int
  meta_array = 0
  meta_entry = ""
  result = .int
  result = 0

  if module <= 0 then return 0
  if address < 0 then return 0

  do a = address to 0 by -1
    assembler metaloaddata meta_array, module, a
    do i = 1 to meta_array
      assembler linkattr1 meta_entry, meta_array, i
      if meta_entry = wanted then do
        assembler linkattr1 result, meta_entry, attr
        return result
      end
    end
  end

  return 0
