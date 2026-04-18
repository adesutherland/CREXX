options levelb
namespace rxcptest expose newtokens pushidentifier pushexitkeyword pushstring pushint pushoperator pushbracket pushcomma check_equal check_contains check_true find_keyword find_binding find_descriptor_import descriptor_has_flag join_result_lines report_result

import rxcp
import rxfnsb

newtokens: procedure = .token[]
    tokens = .token[]
    return tokens

pushidentifier: procedure = .int
    arg expose tokens = .token[], text = .string, value_type = ".unknown", value_dims = 0
    index = tokens[0] + 1
    tokens[index] = .token(0, 0, text, 1, 1, length(text), "exit_test", 0, value_type, "identifier", value_dims)
    return index

pushexitkeyword: procedure = .int
    arg expose tokens = .token[], text = .string
    index = tokens[0] + 1
    tokens[index] = .token(0, 0, text, 1, 1, length(text), "exit_test", 0, ".unknown", "exit_keyword", 0)
    return index

pushstring: procedure = .int
    arg expose tokens = .token[], text = .string
    index = tokens[0] + 1
    tokens[index] = .token(0, 0, text, 1, 1, length(text), "exit_test", 0, ".string", "string_literal", 0)
    return index

pushint: procedure = .int
    arg expose tokens = .token[], text = .string
    index = tokens[0] + 1
    tokens[index] = .token(0, 0, text, 1, 1, length(text), "exit_test", 0, ".int", "int_literal", 0)
    return index

pushoperator: procedure = .int
    arg expose tokens = .token[], text = .string
    index = tokens[0] + 1
    tokens[index] = .token(0, 0, text, 1, 1, length(text), "exit_test", 0, "", "operator", 0)
    return index

pushbracket: procedure = .int
    arg expose tokens = .token[], text = .string
    index = tokens[0] + 1
    tokens[index] = .token(0, 0, text, 1, 1, length(text), "exit_test", 0, "", "bracket", 0)
    return index

pushcomma: procedure = .int
    arg expose tokens = .token[]
    index = tokens[0] + 1
    tokens[index] = .token(0, 0, ",", 1, 1, 1, "exit_test", 0, "", "comma", 0)
    return index

check_equal: procedure = .int
    arg label = .string, expected = .string, actual = .string
    if expected = actual then return 1
    say "FAIL " || label || " expected=<" || expected || "> actual=<" || actual || ">"
    return 0

check_contains: procedure = .int
    arg label = .string, text = .string, fragment = .string
    if pos(fragment, text) > 0 then return 1
    say "FAIL " || label || " missing fragment=<" || fragment || "> text=<" || text || ">"
    return 0

check_true: procedure = .int
    arg label = .string, condition = 0, detail = .string
    if condition \= 0 then return 1
    say "FAIL " || label || " " || detail
    return 0

find_keyword: procedure = .int
    arg plan = .exitplan, keyword_text = .string
    do i = 1 to plan.get_keyword_count()
        keyword = plan.get_keyword(i)
        if upper(keyword.get_keyword_text()) = upper(keyword_text) then return i
    end
    return 0

find_binding: procedure = .int
    arg plan = .exitplan, internal_name = .string
    do i = 1 to plan.get_binding_count()
        binding = plan.get_binding(i)
        if binding.get_internal_name() = internal_name then return i
    end
    return 0

find_descriptor_import: procedure = .int
    arg desc = .exitdescriptor, namespace_name = .string
    do i = 1 to desc.get_default_import_count()
        import_plan = desc.get_default_import(i)
        if upper(import_plan.get_namespace_name()) = upper(namespace_name) then return i
    end
    return 0

descriptor_has_flag: procedure = .int
    arg desc = .exitdescriptor, flag = .string
    return desc.has_flag(flag)

join_result_lines: procedure = .string
    arg result = .exitresult
    text = ""
    do i = 1 to result.get_replacement_line_count()
        if text \= "" then text = text || " "
        text = text || result.get_replacement_line(i)
    end
    return text

report_result: procedure
    arg failures = 0
    if failures = 0 then say "SUCCESS"
    else say "FAIL count=" || failures
