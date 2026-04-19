options levelb
import rxcptest
import rxcpexits
import rxcp

main: procedure
  failures = .int
  failures = 0

  failures = failures + test_descriptor()
  failures = failures + test_set_environment()
  failures = failures + test_explicit_address()
  failures = failures + test_implicit_address()
  failures = failures + test_implicit_string_no_warning()

  call report_result failures
  return

test_descriptor: procedure = .int
  failures = .int
  addr = .addressexit(1000)
  desc = .exitdescriptor
  failures = 0

  desc = addr.describe()
  if check_equal("address protocol version", "2", desc.get_protocol_version()) = 0 then failures = failures + 1
  if check_true("address certified flag", descriptor_has_flag(desc, "certified") > 0, "missing certified flag") = 0 then failures = failures + 1
  if check_true("address reserved flag", descriptor_has_flag(desc, "reserved_keyword") > 0, "missing reserved flag") = 0 then failures = failures + 1
  if check_true("address implicit flag", descriptor_has_flag(desc, "implicit_command") > 0, "missing implicit flag") = 0 then failures = failures + 1

  return failures

test_set_environment: procedure = .int
  failures = .int
  addr = .addressexit(1003)
  plan = .exitplan
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "ADDRESS"
  call pushidentifier tokens, "cms"

  plan = addr.pre_process(tokens)
  if check_equal("address set env pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  result = addr.process(tokens)
  if check_equal("address set env process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  if check_contains("address set env replacement", join_result_lines(result), "_set_address_environment('cms')") = 0 then failures = failures + 1

  return failures

test_explicit_address: procedure = .int
  failures = .int
  addr = .addressexit(1001)
  plan = .exitplan
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "ADDRESS"
  call pushstring tokens, "'system'"
  call pushidentifier tokens, "cmd", ".string", 0
  call pushidentifier tokens, "INPUT"
  call pushidentifier tokens, "input_lines", ".string[]", 1
  call pushidentifier tokens, "OUTPUT"
  call pushidentifier tokens, "output_lines", ".string[]", 1
  call pushidentifier tokens, "ERROR"
  call pushidentifier tokens, "error_lines", ".string[]", 1
  call pushidentifier tokens, "EXPOSE"
  call pushidentifier tokens, "request_id", ".string", 0

  plan = addr.pre_process(tokens)
  if check_equal("address explicit pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  result = addr.process(tokens)
  if check_equal("address explicit process", "REPLACE", result.get_status()) = 0 then failures = failures + 1

  replacement = join_result_lines(result)
  if check_contains("address explicit env", replacement, "_address('system',cmd") = 0 then failures = failures + 1
  if check_contains("address explicit input", replacement, "_array2redir(input_lines)") = 0 then failures = failures + 1
  if check_contains("address explicit output", replacement, "_redir2array(output_lines)") = 0 then failures = failures + 1
  if check_contains("address explicit error", replacement, "_redir2array(error_lines)") = 0 then failures = failures + 1
  if check_contains("address explicit expose", replacement, "'request_id', request_id") = 0 then failures = failures + 1

  return failures

test_implicit_address: procedure = .int
  failures = .int
  addr = .addressexit(1002)
  plan = .exitplan
  result = .exitresult
  diagnostic = .exitdiagnostic
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "cmd", ".string", 0

  plan = addr.pre_process(tokens)
  if check_equal("address implicit pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  result = addr.process(tokens)
  if check_equal("address implicit process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  if check_contains("address implicit current env", join_result_lines(result), "_address('',cmd,_noredir(),_noredir(),_noredir())") = 0 then failures = failures + 1
  if check_equal("address implicit warning count", "1", result.get_diagnostic_count()) = 0 then failures = failures + 1
  diagnostic = result.get_diagnostic(1)
  if check_equal("address implicit warning severity", "warning", diagnostic.get_severity()) = 0 then failures = failures + 1
  if check_equal("address implicit warning code", "IMPLICIT_ADDRESS", diagnostic.get_code()) = 0 then failures = failures + 1

  return failures

test_implicit_string_no_warning: procedure = .int
  failures = .int
  addr = .addressexit(1004)
  plan = .exitplan
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushstring tokens, "'echo OK'"

  plan = addr.pre_process(tokens)
  if check_equal("address implicit string pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  result = addr.process(tokens)
  if check_equal("address implicit string process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  if check_equal("address implicit string warning count", "0", result.get_diagnostic_count()) = 0 then failures = failures + 1

  return failures
