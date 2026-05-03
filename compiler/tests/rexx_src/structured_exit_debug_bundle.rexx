options levelb
namespace rxcpexits expose nestexit

import rxcp
import rxfnsb

nestexit: class
    _node_id = .int

    *: factory
        arg nid = .int
        _node_id = nid

    describe: method = .exitdescriptor
        desc = .exitdescriptor
        desc = .exitdescriptor("nest")
        return desc

    pre_process: method = .exitplan
        arg tokens = .token[]
        return .exitplan("READY")

    process: method = .exitresult
        arg tokens = .token[]

        result = .exitresult("REPLACE")
        call result.add_replacement_line("do; outer = 0; do i = 1 to 1; if i = 1 then do; outer = 1; end; end; say 'outer=' || outer; end;")
        return result
