options levelb
import rxcptest
import rxcpexits
import rxcp

main: procedure
  failures = .int
  failures = 0

  failures = failures + test_explicit_address()
  failures = failures + test_implicit_address()

  call report_result failures
  return

test_explicit_address: procedure = .int
  failures = .int
  addr = .addressexit(1001)
  plan = .exitplan
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
  if check_equal("address explicit process", "REPLACE", addr.process(tokens)) = 0 then failures = failures + 1

  replacement = addr.get_replacement()
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
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "cmd", ".string", 0

  plan = addr.pre_process(tokens)
  if check_equal("address implicit pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  if check_equal("address implicit process", "REPLACE", addr.process(tokens)) = 0 then failures = failures + 1
  if check_contains("address implicit default env", addr.get_replacement(), "_address('SYSTEM',cmd,_noredir(),_noredir(),_noredir())") = 0 then failures = failures + 1

  return failures
