/* REXX Level B trace runtime internals */
options levelb

namespace rxfnsb expose tracecontext tracecontroller

tracecontext: class
  _signal_code = .int
  _signal_name = .string
  _module = .int
  _address = .int
  _mode = .string
  _line = .int
  _column = .int
  _source = .string
  _source_line = .string
  _has_source = .int
  _closest_source_line = .string
  _asm_line = .string
  _procedure = .string

  *: factory
    arg module = .int, addr = .int, mode = "REXX", signal_code = 31, signal_name = "BREAKPOINT"
    _module = module
    _address = addr
    _mode = upper(mode)
    _signal_code = signal_code
    _signal_name = signal_name
    _line = 0
    _column = 0
    _source = ""
    _source_line = ""
    _closest_source_line = ""
    _asm_line = ""
    _procedure = ""
    _has_source = _trace_exact_source(_module, _address, _line, _column, _source, _source_line)
    _closest_source_line = _trace_closest_source_line(_module, _address)
    _asm_line = _trace_asm_line(_module, _address)
    _procedure = _trace_procedure_name(_module, _address)
    return

  signal_code: method = .int
    return _signal_code

  signal_name: method = .string
    return _signal_name

  module: method = .int
    return _module

  address: method = .int
    return _address

  mode: method = .string
    return _mode

  line: method = .int
    return _line

  column: method = .int
    return _column

  source: method = .string
    return _source

  source_line: method = .string
    return _source_line

  has_source: method = .int
    return _has_source

  closest_source_line: method = .string
    return _closest_source_line

  asm_line: method = .string
    return _asm_line

  procedure: method = .string
    return _procedure

tracecontroller: class
  _mode = .string
  _first_client_module = .int
  _latest_module_only = .int
  _include_runtime = .int

  *: factory
    _mode = "REXX"
    _first_client_module = 1
    _latest_module_only = 0
    _include_runtime = 0
    return

  mode: method = .string
    return _mode

  set_mode: method = .void
    arg mode = .string
    mode = upper(mode)
    if mode <> "ASM" then mode = "REXX"
    _mode = mode
    return

  toggle_mode: method = .string
    if _mode = "REXX" then _mode = "ASM"
    else _mode = "REXX"
    return _mode

  set_first_client_module: method = .void
    arg module = .int
    if module < 1 then module = 1
    _first_client_module = module
    return

  first_client_module: method = .int
    return _first_client_module

  set_latest_module_only: method = .void
    arg enabled = 1
    if enabled = 0 then _latest_module_only = 0
    else _latest_module_only = 1
    return

  latest_module_only: method = .int
    return _latest_module_only

  set_include_runtime: method = .void
    arg enabled = 0
    if enabled = 0 then _include_runtime = 0
    else _include_runtime = 1
    return

  include_runtime: method = .int
    return _include_runtime

  enable_breakpoints: method = .void
    assembler bpon
    return

  disable_breakpoints: method = .void
    assembler bpoff
    return

  context: method = .tracecontext
    arg module = .int, addr = .int, mode = .string
    if mode = "" then mode = _mode
    return .tracecontext(module, addr, mode)

  context_from_interrupt: method = .tracecontext
    arg interrupt = .object
    code = .int
    module = .int
    addr = .int
    name = .string
    code = 31
    name = "BREAKPOINT"
    assembler linkattr1 code, interrupt, 1
    assembler linkattr1 module, interrupt, 2
    assembler linkattr1 addr, interrupt, 3
    assembler linkattr1 name, interrupt, 4
    return .tracecontext(module, addr, _mode, code, name)

  should_trace: method = .int
    arg event = .tracecontext
    module = event.module()
    if module < _first_client_module then return 0
    if _latest_module_only <> 0 then do
      modules = loaded_module_count()
      if module <> modules then return 0
    end
    if _include_runtime = 0 then do
      mod_name = module_name(module)
      if _trace_is_default_excluded(mod_name) then return 0
      if _trace_is_default_excluded(event.procedure()) then return 0
    end
    return 1

  exact_source_line: method = .string
    arg module = .int, addr = .int
    line = .int
    column = .int
    source = .string
    result = .string
    if _trace_exact_source(module, addr, line, column, source, result) = 0 then return ""
    return result

  closest_source_line: method = .string
    arg module = .int, addr = .int
    return _trace_closest_source_line(module, addr)

  asm_line: method = .string
    arg module = .int, addr = .int
    return _trace_asm_line(module, addr)

  procedure_name: method = .string
    arg module = .int, addr = .int
    return _trace_procedure_name(module, addr)

  loaded_module_count: method = .int
    modules = 0
    assembler metaloadedmodules modules
    return modules

  module_name: method = .string
    arg module = .int
    return _trace_module_name(module)

  load_module: method = .int
    arg module_name = .string
    mod_num = 0
    assembler metaloadmodule mod_num, module_name
    return mod_num

  load_library: method = .int
    arg dir = .string
    return load_module(dir || "/library")

  loaded_proc_count: method = .int
    arg module_index = .int
    procs = 0
    if module_index <= 0 then return 0
    assembler metaloadedprocs procs, module_index
    return procs

  loaded_proc_name: method = .string
    arg module_index = .int, proc_index = .int
    procs = 0
    proc = 0
    result = ""
    if module_index <= 0 then return ""
    if proc_index <= 0 then return ""
    assembler metaloadedprocs procs, module_index
    if proc_index > procs then return ""
    assembler linkattr1 proc, procs, proc_index
    assembler linkattr1 result, proc, 1
    return result

  loaded_proc_id: method = .int
    arg module_index = .int, proc_index = .int
    procs = 0
    proc = 0
    result = 0
    if module_index <= 0 then return 0
    if proc_index <= 0 then return 0
    assembler metaloadedprocs procs, module_index
    if proc_index > procs then return 0
    assembler linkattr1 proc, procs, proc_index
    assembler linkattr1 result, proc, 2
    return result

  find_proc: method = .int
    arg mod_num = .int, name = .string
    return _trace_find_proc(mod_num, name)

_trace_exact_source: procedure = .int
  arg module = .int, addr = .int, expose line = .int, expose column = .int, expose source = .string, expose result = .string
  meta_array = 0
  meta_entry = ""
  line = 0
  column = 0
  source = ""
  result = ""
  found = 0
  line_value = 0
  column_value = 0
  source_value = ""

  if module <= 0 then return 0
  if addr < 0 then return 0

  assembler metaloaddata meta_array, module, addr
  do i = 1 to meta_array
    assembler linkattr1 meta_entry, meta_array, i
    if meta_entry = ".meta_src" then do
      assembler linkattr1 line_value, meta_entry, 1
      assembler linkattr1 column_value, meta_entry, 2
      assembler linkattr1 source_value, meta_entry, 3
      line = line_value
      column = column_value
      source = source_value
      result = result || "(" || line_value || ":" || column_value || ") '" || source_value || "'; "
      found = 1
    end
  end

  return found

_trace_closest_source_line: procedure = .string
  arg module = .int, addr = .int
  line = .int
  column = .int
  source = .string
  result = .string

  if module <= 0 then return ""
  if addr < 0 then return ""

  do a = addr to 0 by -1
    if _trace_exact_source(module, a, line, column, source, result) <> 0 then return result
  end

  return ""

_trace_procedure_name: procedure = .string
  arg module = .int, addr = .int
  meta_array = 0
  meta_entry = ""
  result = ""

  if module <= 0 then return ""
  if addr < 0 then return ""

  do a = addr to 0 by -1
    assembler metaloaddata meta_array, module, a
    do i = 1 to meta_array
      assembler linkattr1 meta_entry, meta_array, i
      if meta_entry = ".meta_func" then do
        assembler linkattr1 result, meta_entry, 1
        return result
      end
    end
  end

  return ""

_trace_asm_line: procedure = .string
  arg module = .int, addr = .int
  opcode = 0
  instruction = ""
  description = ""
  no_operands = 0
  op1_type = 0
  op2_type = 0
  op3_type = 0
  instruction_object = 0

  if module <= 0 then return ""
  if addr < 0 then return ""

  assembler metaloadinst opcode, module, addr
  assembler metadecodeinst instruction_object, opcode
  assembler linkattr1 instruction, instruction_object, 2
  assembler linkattr1 description, instruction_object, 3
  assembler linkattr1 no_operands, instruction_object, 4
  assembler linkattr1 op1_type, instruction_object, 5
  assembler linkattr1 op2_type, instruction_object, 6
  assembler linkattr1 op3_type, instruction_object, 7

  return " " right(d2x(opcode), 3, "0") "@" right(d2x(module), 3, "0")":"right(d2x(addr), 4, "0") instruction,
         _trace_operand_description(op1_type, module, addr + 1)","_trace_operand_description(op2_type, module, addr + 2)","_trace_operand_description(op3_type, module, addr + 3),
         "*" description

_trace_operand_description: procedure = .string
  arg code = .int, module = .int, addr = .int
  desc = "unknown"
  int_val = 0
  float_val = 0.0
  string_val = ""

  if code = 0 then desc = ""
  else if code = 1 then do
    assembler metaloadioperand int_val, module, addr
    desc = "@" || right(d2x(int_val), 4, '0')
  end
  else if code = 2 then do
    assembler metaloadioperand int_val, module, addr
    desc = "r" || int_val
  end
  else if code = 3 then do
    assembler metaloadpoperand string_val, module, addr
    desc = string_val || "()"
  end
  else if code = 4 then do
    assembler metaloadioperand int_val, module, addr
    desc = int_val
  end
  else if code = 5 then do
    assembler metaloadfoperand float_val, module, addr
    desc = float_val || "f"
  end
  else if code = 6 then do
    assembler metaloadioperand int_val, module, addr
    desc = int_val || "c"
  end
  else if code = 7 then do
    assembler metaloadsoperand string_val, module, addr
    desc = '"' || string_val || '"'
  end

  return desc

_trace_module_name: procedure = .string
  arg module = .int
  modules = 0
  result = ""

  if module <= 0 then return ""
  assembler metaloadedmodules modules
  if module > modules then return ""
  assembler linkattr1 result, modules, module
  return result

_trace_is_default_excluded: procedure = .int
  arg name = .string
  n = lower(name)
  if n = "" then return 0
  if pos("rxdb", n) > 0 then return 1
  if pos("tracecontroller", n) > 0 then return 1
  if pos("tracecontext", n) > 0 then return 1
  if pos("runtime_signal", n) > 0 then return 1
  if pos("signalaction", n) > 0 then return 1
  if pos("rxcptest", n) > 0 then return 1
  if pos("rxcpexits", n) > 0 then return 1
  if pos("rxcp", n) > 0 then return 1
  if pos("_rxsysb", n) > 0 then return 1
  if pos("/rxfnsb/", n) > 0 then return 1
  if pos("\\rxfnsb\\", n) > 0 then return 1
  return 0

_trace_find_proc: procedure = .int
  arg mod_num = .int, name = .string
  modules = 0
  module = ""
  proc_name = ""
  proc_id = 0
  proc = 0
  procs = 0
  start = 0
  finish = 0

  assembler metaloadedmodules modules
  if mod_num = 0 then do
    start = 1
    finish = modules
  end
  else do
    start = mod_num
    finish = mod_num
  end

  do module_index = start to finish
    assembler linkattr1 module, modules, module_index
    assembler metaloadedprocs procs, module_index
    do j = 1 to procs
      assembler linkattr1 proc, procs, j
      assembler linkattr1 proc_name, proc, 1
      assembler linkattr1 proc_id, proc, 2
      if proc_name = name then return proc_id
    end
  end
  return 0
