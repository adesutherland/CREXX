options levelb
namespace rxcp expose exit_dispatch

exit_dispatch: procedure = .string
    arg tokens = .token[]

    exit_obj = .dumpexit()
    return exit_obj.process(tokens)
