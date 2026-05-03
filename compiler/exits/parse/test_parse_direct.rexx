options levelb
import rxcptest
import rxcpexits
import rxcp

main: procedure
  failures = .int
  failures = 0

  failures = failures + test_descriptor()
  failures = failures + test_parse_plan()
  failures = failures + test_parse_arg_plan()
  failures = failures + test_parse_incomplete_inputs()
  failures = failures + test_parse_suffix_options()
  failures = failures + test_parse_claimed_keywords()

  call report_result failures
  return

test_descriptor: procedure = .int
  failures = .int
  parser = .parseexit(2000)
  desc = .exitdescriptor
  failures = 0

  desc = parser.describe()
  if check_equal("parse protocol version", "2", desc.get_protocol_version()) = 0 then failures = failures + 1
  if check_true("parse certified flag", descriptor_has_flag(desc, "certified") > 0, "missing certified flag") = 0 then failures = failures + 1
  if check_true("parse reserved flag", descriptor_has_flag(desc, "reserved_keyword") > 0, "missing reserved flag") = 0 then failures = failures + 1
  if check_true("parse descriptor import", find_descriptor_import(desc, "rxfnsb") > 0, "missing rxfnsb import") = 0 then failures = failures + 1

  return failures

test_parse_plan: procedure = .int
  failures = .int
  parser = .parseexit(2001)
  plan = .exitplan
  result = .exitresult
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
     binding = plan.get_binding(abc_bind)
     if check_equal("parse abc provenance", "parse_into", binding.get_provenance()) = 0 then failures = failures + 1
  end
  if first_bind > 0 then do
     binding = plan.get_binding(first_bind)
     if check_equal("parse first provenance", "parse_target", binding.get_provenance()) = 0 then failures = failures + 1
  end
  if var_kw > 0 then do
     keyword = plan.get_keyword(var_kw)
     if check_equal("parse var role", "parse_source", keyword.get_keyword_role()) = 0 then failures = failures + 1
  end

  result = parser.process(tokens)
  if check_equal("parse process", "REPLACE", result.get_status()) = 0 then failures = failures + 1

  replacement = join_result_lines(result)
  if check_contains("parse source rewrite", replacement, "_source=upper(fred)") = 0 then failures = failures + 1
  if check_contains("parse runtime call", replacement, "abc=parseExec(_source,") = 0 then failures = failures + 1
  if check_contains("parse first assignment", replacement, "first=abc[1]") = 0 then failures = failures + 1
  if check_contains("parse second assignment", replacement, "second=abc[2]") = 0 then failures = failures + 1

  return failures

test_parse_arg_plan: procedure = .int
  failures = .int
  parser = .parseexit(2005)
  plan = .exitplan
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "PARSE"
  call pushidentifier tokens, "ARG"
  call pushidentifier tokens, "execName", ".string", 0
  call pushstring tokens, "'.'"
  call pushidentifier tokens, "extension", ".string", 0

  plan = parser.pre_process(tokens)
  if check_equal("parse arg pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  if check_true("parse arg keyword", find_keyword(plan, "ARG") > 0, "keyword claim missing") = 0 then failures = failures + 1
  if check_equal("parse arg binding count", "2", plan.get_binding_count()) = 0 then failures = failures + 1
  if check_true("parse arg execName binding", find_binding(plan, "execName") > 0, "binding missing") = 0 then failures = failures + 1
  if check_true("parse arg extension binding", find_binding(plan, "extension") > 0, "binding missing") = 0 then failures = failures + 1

  result = parser.process(tokens)
  if check_equal("parse arg process", "REPLACE", result.get_status()) = 0 then failures = failures + 1

  replacement = join_result_lines(result)
  if check_contains("parse arg source count", replacement, "__rxcpx_parse_arg_count=arg()") = 0 then failures = failures + 1
  if check_contains("parse arg source loop", replacement, "__rxcpx_parse_arg_source=__rxcpx_parse_arg_source || arg[__rxcpx_parse_arg_ix]") = 0 then failures = failures + 1
  if check_contains("parse arg block return", replacement, "leave with __rxcpx_parse_arg_source") = 0 then failures = failures + 1
  if check_contains("parse arg runtime call", replacement, "_parseResult=parseExec(_source,") = 0 then failures = failures + 1
  if check_contains("parse arg first assignment", replacement, "execName=_parseResult[1]") = 0 then failures = failures + 1
  if check_contains("parse arg second assignment", replacement, "extension=_parseResult[2]") = 0 then failures = failures + 1

  return failures

test_parse_incomplete_inputs: procedure = .int
  failures = .int
  parser = .parseexit(2004)
  plan = .exitplan
  diagnostic = .exitdiagnostic
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "PARSE"
  call pushidentifier tokens, "INTO"

  plan = parser.pre_process(tokens)
  if check_equal("parse into pre_process", "ERROR", plan.get_status()) = 0 then failures = failures + 1
  if check_equal("parse into diagnostic count", "1", plan.get_diagnostic_count()) = 0 then failures = failures + 1
  if plan.get_diagnostic_count() > 0 then do
     diagnostic = plan.get_diagnostic(1)
     if check_equal("parse into diagnostic code", "MISSING_ARGUMENTS", diagnostic.get_code()) = 0 then failures = failures + 1
     if check_equal("parse into diagnostic message", "PARSE INTO requires a target variable", diagnostic.get_message()) = 0 then failures = failures + 1
  end

  tokens = newtokens()
  call pushidentifier tokens, "PARSE"
  call pushidentifier tokens, "VALUE"

  plan = parser.pre_process(tokens)
  if check_equal("parse value pre_process", "ERROR", plan.get_status()) = 0 then failures = failures + 1
  if check_equal("parse value diagnostic count", "1", plan.get_diagnostic_count()) = 0 then failures = failures + 1
  if plan.get_diagnostic_count() > 0 then do
     diagnostic = plan.get_diagnostic(1)
     if check_equal("parse value diagnostic code", "MISSING_ARGUMENTS", diagnostic.get_code()) = 0 then failures = failures + 1
     if check_equal("parse value diagnostic message", "PARSE VALUE requires a source expression", diagnostic.get_message()) = 0 then failures = failures + 1
  end

  tokens = newtokens()
  call pushidentifier tokens, "PARSE"

  plan = parser.pre_process(tokens)
  if check_equal("parse missing source pre_process", "ERROR", plan.get_status()) = 0 then failures = failures + 1
  if check_equal("parse missing source diagnostic count", "1", plan.get_diagnostic_count()) = 0 then failures = failures + 1
  if plan.get_diagnostic_count() > 0 then do
     diagnostic = plan.get_diagnostic(1)
     if check_equal("parse missing source diagnostic code", "MISSING_ARGUMENTS", diagnostic.get_code()) = 0 then failures = failures + 1
     if check_equal("parse missing source diagnostic message", "PARSE requires arguments", diagnostic.get_message()) = 0 then failures = failures + 1
  end

  return failures

test_parse_suffix_options: procedure = .int
  failures = .int
  parser = .parseexit(2002)
  plan = .exitplan
  result = .exitresult
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

  result = parser.process(tokens)
  if check_equal("parse suffix process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  replacement = join_result_lines(result)
  if check_contains("parse suffix into call", replacement, "parsed=parseExec(_source,") = 0 then failures = failures + 1
  if check_contains("parse suffix template quoting", replacement, ",' left "","" right',0)") = 0 then failures = failures + 1
  if check_contains("parse suffix trim assign", replacement, "left=strip(parsed[1])") = 0 then failures = failures + 1
  if check_contains("parse suffix trim assign 2", replacement, "right=strip(parsed[2])") = 0 then failures = failures + 1

  return failures

test_parse_claimed_keywords: procedure = .int
  failures = .int
  parser = .parseexit(2003)
  plan = .exitplan
  result = .exitresult
  failures = 0

  tokens = newtokens()
  call pushidentifier tokens, "PARSE"
  call pushexitkeyword tokens, "UPPER"
  call pushexitkeyword tokens, "VALUE"
  call pushidentifier tokens, "src", ".string", 0
  call pushexitkeyword tokens, "WITH"
  call pushidentifier tokens, "left", ".string", 0
  call pushstring tokens, '","'
  call pushidentifier tokens, "right", ".string", 0
  call pushexitkeyword tokens, "TRIM"
  call pushexitkeyword tokens, "INTO"
  call pushidentifier tokens, "parsed", ".string[]", 1

  plan = parser.pre_process(tokens)
  if check_equal("parse claimed pre_process", "READY", plan.get_status()) = 0 then failures = failures + 1
  if check_true("parse claimed trim keyword", find_keyword(plan, "TRIM") > 0, "keyword claim missing") = 0 then failures = failures + 1
  if check_true("parse claimed into keyword", find_keyword(plan, "INTO") > 0, "keyword claim missing") = 0 then failures = failures + 1

  result = parser.process(tokens)
  if check_equal("parse claimed process", "REPLACE", result.get_status()) = 0 then failures = failures + 1
  replacement = join_result_lines(result)
  if check_contains("parse claimed source rewrite", replacement, "_source=upper(src)") = 0 then failures = failures + 1
  if check_contains("parse claimed runtime call", replacement, "parsed=parseExec(_source,") = 0 then failures = failures + 1
  if check_contains("parse claimed trim assign", replacement, "left=strip(parsed[1])") = 0 then failures = failures + 1
  if check_contains("parse claimed trim assign 2", replacement, "right=strip(parsed[2])") = 0 then failures = failures + 1

  return failures
