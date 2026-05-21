options levelb
import rxcptest
import rxcpexits
import rxcp

main: procedure
  failures = .int
  failures = 0

  failures = failures + test_descriptor()
  failures = failures + test_modes()
  failures = failures + test_pre_process_keywords()
  failures = failures + test_errors()

  call report_result failures
  return

test_descriptor: procedure = .int
  failures = .int
  tx = .traceexit(3000)
  desc = .exitdescriptor
  failures = 0

  desc = tx.describe()
  if check_equal("trace protocol version", "2", desc.get_protocol_version()) = 0 then failures = failures + 1
  if check_true("trace certified flag", descriptor_has_flag(desc, "certified") > 0, "missing certified flag") = 0 then failures = failures + 1
  if check_true("trace reserved flag", descriptor_has_flag(desc, "reserved_keyword") > 0, "missing reserved flag") = 0 then failures = failures + 1
  if check_true("trace rxfnsb import", find_descriptor_import(desc, "rxfnsb") > 0, "missing rxfnsb import") = 0 then failures = failures + 1

  return failures

test_modes: procedure = .int
  failures = .int
  tx = .traceexit(3001)
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushexitkeyword tokens, "TRACE"
  call pushidentifier tokens, "normal"
  result = tx.process(tokens)
  if check_equal("trace normal process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  replacement = join_result_lines(result)
  if check_contains("trace normal mode", replacement, 'call _trace_set("REXX")') = 0 then failures = failures + 1
  if check_contains("trace normal handler", replacement, 'assembler sigcall __rxtrace_handler(),"BREAKPOINT"') = 0 then failures = failures + 1
  if check_contains("trace normal bpon", replacement, 'assembler bpon') = 0 then failures = failures + 1

  tokens = newtokens()
  call pushexitkeyword tokens, "TRACE"
  call pushidentifier tokens, "asm"
  result = tx.process(tokens)
  if check_equal("trace asm process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  replacement = join_result_lines(result)
  if check_contains("trace asm mode", replacement, 'call _trace_set("ASM")') = 0 then failures = failures + 1

  tokens = newtokens()
  call pushexitkeyword tokens, "TRACE"
  call pushidentifier tokens, "off"
  result = tx.process(tokens)
  if check_equal("trace off process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  replacement = join_result_lines(result)
  if check_contains("trace off mode", replacement, 'call _trace_set("OFF")') = 0 then failures = failures + 1
  if check_contains("trace off bpoff", replacement, 'assembler bpoff') = 0 then failures = failures + 1

  return failures

test_pre_process_keywords: procedure = .int
  failures = .int
  tx = .traceexit(3002)
  plan = .exitplan
  failures = 0

  tokens = newtokens()
  call pushexitkeyword tokens, "TRACE"
  call pushidentifier tokens, "rexx"

  plan = tx.pre_process(tokens)
  if check_equal("trace pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  if check_true("trace keyword claim", find_keyword(plan, "rexx") > 0, "missing trace option keyword") = 0 then failures = failures + 1
  if check_equal("trace helper count", "1", plan.get_helper_count()) = 0 then failures = failures + 1
  if plan.get_helper_count() > 0 then do
    helper = plan.get_helper(1)
    if check_contains("trace helper delegates", helper.get_line(3), "_trace_handler(__rxtrace_raw)") = 0 then failures = failures + 1
  end

  return failures

test_errors: procedure = .int
  failures = .int
  tx = .traceexit(3003)
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushexitkeyword tokens, "TRACE"
  result = tx.process(tokens)
  if check_equal("trace missing option status", "ERROR", result.get_status()) = 0 then failures = failures + 1

  tokens = newtokens()
  call pushexitkeyword tokens, "TRACE"
  call pushidentifier tokens, "all"
  result = tx.process(tokens)
  if check_equal("trace unknown option status", "ERROR", result.get_status()) = 0 then failures = failures + 1
  if result.get_diagnostic_count() > 0 then do
    diagnostic = result.get_diagnostic(1)
    if check_equal("trace unknown option code", "TRACE_UNKNOWN_OPTION", diagnostic.get_code()) = 0 then failures = failures + 1
  end

  tokens = newtokens()
  call pushexitkeyword tokens, "TRACE"
  call pushidentifier tokens, "rexx"
  call pushidentifier tokens, "extra"
  result = tx.process(tokens)
  if check_equal("trace extra operand status", "ERROR", result.get_status()) = 0 then failures = failures + 1

  return failures
