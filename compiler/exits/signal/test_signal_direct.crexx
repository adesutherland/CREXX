options levelb
import rxcptest
import rxcpexits
import rxcp

main: procedure
  failures = .int
  failures = 0

  failures = failures + test_descriptor()
  failures = failures + test_compact_raise()
  failures = failures + test_factory_raise()
  failures = failures + test_handler()
  failures = failures + test_diagnostic_mapper()
  failures = failures + test_off()

  call report_result failures
  return

test_descriptor: procedure = .int
  failures = .int
  sx = .signalexit(2000)
  desc = .exitdescriptor
  failures = 0

  desc = sx.describe()
  if check_equal("signal protocol version", "2", desc.get_protocol_version()) = 0 then failures = failures + 1
  if check_true("signal certified flag", descriptor_has_flag(desc, "certified") > 0, "missing certified flag") = 0 then failures = failures + 1
  if check_true("signal reserved flag", descriptor_has_flag(desc, "reserved_keyword") > 0, "missing reserved flag") = 0 then failures = failures + 1
  if check_true("signal diagnostic mapper flag", descriptor_has_flag(desc, "diagnostic_mapper") > 0, "missing diagnostic mapper flag") = 0 then failures = failures + 1
  if check_true("signal rxfnsb import", find_descriptor_import(desc, "rxfnsb") > 0, "missing rxfnsb import") = 0 then failures = failures + 1

  return failures

test_compact_raise: procedure = .int
  failures = .int
  sx = .signalexit(2001)
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushexitkeyword tokens, "SIGNAL"
  call pushidentifier tokens, "conversion_error"
  call pushstring tokens, '"bad input"'

  result = sx.process(tokens)
  if check_equal("signal compact process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  if check_contains("signal compact replacement", join_result_lines(result), 'assembler signal "CONVERSION_ERROR","bad input"') = 0 then failures = failures + 1

  return failures

test_factory_raise: procedure = .int
  failures = .int
  sx = .signalexit(2002)
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushexitkeyword tokens, "SIGNAL"
  call pushidentifier tokens, ".signal"
  call pushbracket tokens, "("
  call pushidentifier tokens, "sig_name"
  call pushcomma tokens
  call pushidentifier tokens, "payload"
  call pushbracket tokens, ")"

  result = sx.process(tokens)
  if check_equal("signal factory process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  replacement = join_result_lines(result)
  if check_contains("signal factory name temp", replacement, "__rxsignal_name=sig_name") = 0 then failures = failures + 1
  if check_contains("signal factory payload temp", replacement, "__rxsignal_payload=payload") = 0 then failures = failures + 1
  if check_contains("signal factory assembler", replacement, "assembler signal __rxsignal_name,__rxsignal_payload") = 0 then failures = failures + 1

  return failures

test_handler: procedure = .int
  failures = .int
  sx = .signalexit(2003)
  plan = .exitplan
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushexitkeyword tokens, "SIGNAL"
  call pushidentifier tokens, "on"
  call pushidentifier tokens, "error"
  call pushcomma tokens
  call pushidentifier tokens, "syntax"
  call pushidentifier tokens, "call"
  call pushidentifier tokens, "handle_problem"

  plan = sx.pre_process(tokens)
  if check_equal("signal handler pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  if check_equal("signal handler helper count", "1", plan.get_helper_count()) = 0 then failures = failures + 1
  if plan.get_helper_count() > 0 then do
    helper = plan.get_helper(1)
    if check_contains("signal handler helper token map", helper.get_line(6), "{7}(__rxsignal_condition)") = 0 then failures = failures + 1
  end

  result = sx.process(tokens)
  if check_equal("signal handler process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  replacement = join_result_lines(result)
  if check_contains("signal handler sigcalla", replacement, 'assembler sigcalla __rxsignal_handle_problem(),"ERROR"') = 0 then failures = failures + 1

  return failures

test_diagnostic_mapper: procedure = .int
  failures = .int
  sx = .signalexit(2005)
  diagnostic = .exitdiagnostic
  failures = 0

  diagnostic = sx.map_diagnostic("BAD_CONVERSION", "", "handle_other", "__rxsignal_handle_other:")
  if check_equal("signal bad signature severity", "error", diagnostic.get_severity()) = 0 then failures = failures + 1
  if check_equal("signal bad signature code", "SIGNAL_HANDLER_BAD_SIGNATURE", diagnostic.get_code()) = 0 then failures = failures + 1
  if check_equal("signal bad signature message", "handle_other", diagnostic.get_message()) = 0 then failures = failures + 1

  diagnostic = sx.map_diagnostic("FUNCTION_NOT_FOUND", "", "missing_handler", "__rxsignal_missing_handler:")
  if check_equal("signal missing handler code", "SIGNAL_HANDLER_NOT_FOUND", diagnostic.get_code()) = 0 then failures = failures + 1
  if check_equal("signal missing handler message", "missing_handler", diagnostic.get_message()) = 0 then failures = failures + 1

  diagnostic = sx.map_diagnostic("BAD_CONVERSION", "", "other", "not_signal_helper")
  if check_equal("signal mapper ignores other helpers", "", diagnostic.get_code()) = 0 then failures = failures + 1

  return failures

test_off: procedure = .int
  failures = .int
  sx = .signalexit(2006)
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushexitkeyword tokens, "SIGNAL"
  call pushidentifier tokens, "off"
  call pushidentifier tokens, "posix_usr1"

  result = sx.process(tokens)
  if check_equal("signal off process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  if check_contains("signal off replacement", join_result_lines(result), 'assembler sigignore "POSIX_USR1"') = 0 then failures = failures + 1

  return failures
