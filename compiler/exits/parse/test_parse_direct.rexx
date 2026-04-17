options levelb
import rxcptest
import rxcpexits
import rxcp

main: procedure
  failures = .int
  failures = 0

  failures = failures + test_parse_plan()
  failures = failures + test_parse_suffix_options()

  call report_result failures
  return

test_parse_plan: procedure = .int
  failures = .int
  parser = .parseexit(2001)
  plan = .exitplan
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "PARSE"
  call pushidentifier tokens, "UPPER"
  call pushidentifier tokens, "INTO"
  call pushidentifier tokens, "abc", ".string[]", 1
  call pushidentifier tokens, "VAR"
  call pushidentifier tokens, "fred", ".string", 0
  call pushidentifier tokens, "first", ".string", 0
  call pushidentifier tokens, "second", ".string", 0

  plan = parser.pre_process(tokens)
  if check_equal("parse pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  if check_equal("parse keyword count", "3", plan.get_keyword_count()) = 0 then failures = failures + 1
  if check_equal("parse binding count", "3", plan.get_binding_count()) = 0 then failures = failures + 1

  upper_kw = find_keyword(plan, "UPPER")
  into_kw = find_keyword(plan, "INTO")
  var_kw = find_keyword(plan, "VAR")
  abc_bind = find_binding(plan, "abc")
  first_bind = find_binding(plan, "first")
  second_bind = find_binding(plan, "second")

  if check_true("parse upper keyword", upper_kw > 0, "keyword claim missing") = 0 then failures = failures + 1
  if check_true("parse into keyword", into_kw > 0, "keyword claim missing") = 0 then failures = failures + 1
  if check_true("parse var keyword", var_kw > 0, "keyword claim missing") = 0 then failures = failures + 1
  if check_true("parse abc binding", abc_bind > 0, "binding missing") = 0 then failures = failures + 1
  if check_true("parse first binding", first_bind > 0, "binding missing") = 0 then failures = failures + 1
  if check_true("parse second binding", second_bind > 0, "binding missing") = 0 then failures = failures + 1

  if abc_bind > 0 then do
     if check_equal("parse abc provenance", "parse_into", plan.get_binding_provenance(abc_bind)) = 0 then failures = failures + 1
  end
  if first_bind > 0 then do
     if check_equal("parse first provenance", "parse_target", plan.get_binding_provenance(first_bind)) = 0 then failures = failures + 1
  end
  if var_kw > 0 then do
     if check_equal("parse var role", "parse_source", plan.get_keyword_role(var_kw)) = 0 then failures = failures + 1
  end

  if check_equal("parse process", "REPLACE", parser.process(tokens)) = 0 then failures = failures + 1

  replacement = parser.get_replacement()
  if check_contains("parse source rewrite", replacement, "_source=upper(fred)") = 0 then failures = failures + 1
  if check_contains("parse runtime call", replacement, "abc=parseExec(_source,") = 0 then failures = failures + 1
  if check_contains("parse first assignment", replacement, "first=abc[1]") = 0 then failures = failures + 1
  if check_contains("parse second assignment", replacement, "second=abc[2]") = 0 then failures = failures + 1

  return failures

test_parse_suffix_options: procedure = .int
  failures = .int
  parser = .parseexit(2002)
  plan = .exitplan
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "PARSE"
  call pushidentifier tokens, "UPPER"
  call pushidentifier tokens, "VALUE"
  call pushidentifier tokens, "src", ".string", 0
  call pushidentifier tokens, "WITH"
  call pushidentifier tokens, "left", ".string", 0
  call pushstring tokens, '","'
  call pushidentifier tokens, "right", ".string", 0
  call pushidentifier tokens, "TRIM"
  call pushidentifier tokens, "INTO"
  call pushidentifier tokens, "parsed", ".string[]", 1

  plan = parser.pre_process(tokens)
  if check_equal("parse suffix pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  if check_true("parse suffix trim keyword", find_keyword(plan, "TRIM") > 0, "keyword claim missing") = 0 then failures = failures + 1
  if check_true("parse suffix into keyword", find_keyword(plan, "INTO") > 0, "keyword claim missing") = 0 then failures = failures + 1
  parsed_bind = find_binding(plan, "parsed")
  if check_true("parse suffix into binding", parsed_bind > 0, "binding missing") = 0 then failures = failures + 1

  if check_equal("parse suffix process", "REPLACE", parser.process(tokens)) = 0 then failures = failures + 1
  replacement = parser.get_replacement()
  if check_contains("parse suffix into call", replacement, "parsed=parseExec(_source,") = 0 then failures = failures + 1
  if check_contains("parse suffix trim assign", replacement, "left=strip(parsed[1])") = 0 then failures = failures + 1
  if check_contains("parse suffix trim assign 2", replacement, "right=strip(parsed[2])") = 0 then failures = failures + 1

  return failures
