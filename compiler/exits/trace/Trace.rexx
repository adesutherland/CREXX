options levelb
namespace rxcpexits expose traceexit

import rxcp
import rxfnsb

traceexit: class
    _id = .int

    *: factory
        arg id = 0
        _id = id
        return

    describe: method = .exitdescriptor
        desc = .exitdescriptor("TRACE")
        call desc.add_flag("certified")
        call desc.add_flag("reserved_keyword")
        call desc.add_import("rxfnsb", "trace", "")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        plan = .exitplan("READY")
        start = firstPayloadToken(tokens)
        if start > 0 & start <= tokens.0 then do
            mode = canonicalTraceMode(tokens[start].get_text())
            if mode <> "" then do
                call plan.add_keyword(start, tokens[start].get_text(), "trace_option", "trace")
                if mode <> "OFF" then call plan.add_helper(makeTraceHelper())
            end
        end
        return plan

    process: method = .exitresult
        arg tokens = .token[]
        start = firstPayloadToken(tokens)
        if start = 0 then return error_result(1, "TRACE_REQUIRES_OPTION")
        if start <> tokens.0 then return error_result(start + 1, "TRACE_UNEXPECTED_OPERAND")

        mode = canonicalTraceMode(tokens[start].get_text())
        if mode = "" then return error_result(start, "TRACE_UNKNOWN_OPTION")

        result = .exitresult("REPLACE")
        if mode = "OFF" then do
            call result.add_replacement_line("assembler bpoff")
            call result.add_replacement_line("call _trace_set(""OFF"")")
        end
        else do
            call result.add_replacement_line("call _trace_set(""" || mode || """)")
            call result.add_replacement_line("assembler sigcall __rxtrace_handler(),""BREAKPOINT""")
            call result.add_replacement_line("assembler bpon")
        end
        return result

firstPayloadToken: procedure = .int
    arg tokens = .token[]
    if tokens.0 < 1 then return 0
    if upper(tokens[1].get_text()) = "TRACE" then do
        if tokens.0 < 2 then return 0
        return 2
    end
    return 1

canonicalTraceMode: procedure = .string
    arg text = .string
    mode = upper(strip(text))
    if mode = "NORMAL" then return "REXX"
    if mode = "REXX" then return "REXX"
    if mode = "ASM" then return "ASM"
    if mode = "OFF" then return "OFF"
    return ""

makeTraceHelper: procedure = .helperplan
    helper = .helperplan("__rxtrace_handler", "file_tail", "__rxtrace_handler", "")
    call helper.add_line("__rxtrace_handler: procedure = .int")
    call helper.add_line("  arg expose __rxtrace_raw = .trace_interrupt_raw")
    call helper.add_line("  return _trace_handler(__rxtrace_raw)")
    return helper

error_result: procedure = .exitresult
    arg token_index = .int, message = .string
    result = .exitresult("ERROR")
    call result.set_error(token_index, message)
    return result
