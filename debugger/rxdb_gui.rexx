/*
   Prototype CREXX Debugger text UI helpers
*/
options levelb
namespace rxdbgui expose rxdbtextgui
import rxfnsb

rxdbtextgui: class
  _plain = .int
  _step_batch = .int
  _step_batch_label = .string
  _esc = .string
  _green = .string
  _reset = .string
  _topleft = .string
  _clear = .string
  _clearline = .string
  _cursorup = .string
  _line2 = .string
  _bottom = .string

  *: factory
    arg mode = "ansi"
    mode = lower(mode)
    _plain = 0
    _step_batch = 1
    _step_batch_label = "1"
    if mode = "text" | mode = "plain" | mode = "llm" then _plain = 1
    if mode = "llm" then do
      _step_batch = 50
      _step_batch_label = "50"
    end
    _esc = '1B'x
    if _plain = 0 then do
      _green = _esc"[32m"
      _reset = _esc"[0m"
      _topleft = _esc"[1;1H"
      _clear = _esc"[2J"
      _clearline = _esc"[2K"
      _cursorup = _esc"[A"
      _line2 = _esc"[2;1H"
      _bottom = _esc"[99;1H" || _esc"[A"
    end
    else do
      _green = ""
      _reset = ""
      _topleft = ""
      _clear = ""
      _clearline = ""
      _cursorup = ""
      _line2 = ""
      _bottom = ""
    end
    return

  is_plain: method = .int
    return _plain

  step_batch: method = .int
    return _step_batch

  green: method = .string
    return _green

  reset: method = .string
    return _reset

  clear_line: method = .string
    return _clearline

  cursor_up: method = .string
    return _cursorup

  bottom: method = .string
    return _bottom

  banner: method = .void
    batch = .int
    batch = _step_batch
    batch_label = .string
    batch_label = _step_batch_label
    if _plain = 0 then say _green"RXDB Version 0.2.0"
    else say "RXDB Version 0.2.0 [text]"
    if batch > 1 then say "RXDB text mode: ENTER steps " || batch_label || " trace events before prompting again."
    say ""
    return

  usage: method = .void
    say "RXDB - REXX Debugger"
    say "Usage: rxdb [text|plain|llm]"
    return

  clear_run_screen: method = .void
    if _plain = 0 then say _clear || _topleft
    else say "RXDB run:"
    return

  non_running_prompt: method = .void
    arg mode = .string
    say "Non-running state command (h=help) - Mode="mode":"
    return

  print_non_running_help: method = .void
    say 'help:'
    say '  q        - Quit'
    say '  m        - Toggle mode (REXX/ASM)'
    say '  e        - Print exposed procedures in modules to be debugged'
    say '  a        - Print exposed procedures (all modules)'
    say '  r        - Run loaded procedure'
    say '  l rxbin  - Load rxbin module file {rxbin}'
    return

  render_step_header: method = .void
    arg last_instruction = .string, next_instruction = .string, mode = .string
    if _plain = 0 then do
      say _topleft || _clearline || _green || last_instruction
      say _line2 || _clearline || _green">"
      say _clearline || next_instruction
      say _clearline
    end
    else do
      say "RXDB step Mode="mode
      if last_instruction <> "" then say "last: " || last_instruction
      say "next: " || next_instruction
    end
    return

  running_prompt: method = .void
    arg mode = .string
    batch = .int
    batch = _step_batch
    batch_label = .string
    batch_label = _step_batch_label
    if _plain = 0 then do
      say _clearline
      say _clearline"Running state command (ENTER=step, h=help) - Mode="mode":"
      say _clearline || _cursorup
    end
    else if batch > 1 then say "Running state command (ENTER=step " || batch_label || ", h=help) - Mode="mode":"
    else say "Running state command (ENTER=step, h=help) - Mode="mode":"
    return

  print_running_help: method = .void
    batch = .int
    batch = _step_batch
    batch_label = .string
    batch_label = _step_batch_label
    say _clearline'help:'
    if batch > 1 then say _clearline'  ENTER    - Step the next ' || batch_label || ' trace events'
    else say _clearline'  ENTER    - Step to next instruction'
    say _clearline'  q        - Quit'
    say _clearline'  w n ...  - Watch regs (numbers) or variables (names)'
    say _clearline'  e        - Print exposed procedures in module being debugged'
    say _clearline'  a        - Print exposed procedures (all modules)'
    say _clearline'  m        - Toggle mode (REXX/ASM)'
    return

  print_procs: method = .void
    arg controller = .tracecontroller, mod_num = .int
    modules = .int
    module = .string
    procs = .int
    proc_name = .string
    proc_id = .int
    start = .int
    finish = .int

    modules = controller.loaded_module_count()
    if mod_num = 0 then do
      say "There are" modules "modules"
      start = 1
      finish = modules
    end
    else do
      start = mod_num
      finish = mod_num
    end

    do i = start to finish
      module = controller.module_name(i)
      say "Module" i module
      procs = controller.loaded_proc_count(i)
      do j = 1 to procs
        proc_name = controller.loaded_proc_name(i, j)
        proc_id = controller.loaded_proc_id(i, j)
        say "Procedure" proc_name '@' proc_id
      end
    end
    return

  exit_line: method = .void
    arg text = "RXDB Exiting"
    if _plain = 0 then say text || _reset
    else say text
    return
