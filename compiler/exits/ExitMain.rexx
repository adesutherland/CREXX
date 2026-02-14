options levelb
namespace rxcp expose exit_dispatch

exit_dispatch: procedure = .string
    arg tokens = .token[1 to *]
    exit_obj = .DumpExit()
    return exit_obj.process(tokens)
