options levelb
namespace rxcp expose DumpExit

DumpExit: class
    *: factory
        return

    process: method = .string
        arg tokens = .token[]

        if tokens.0 < 1 then return ""
        t = tokens.1

        cmd = t.get_text()
        if cmd \= "dump" then return ""

        out_code = "options levelb; do; "
        do i = 2 to tokens.0
            t_text = tokens[i].get_text()
            if t_text \= "" then do
                out_code = out_code || "say '" || t_text || "=' || " || t_text || ";"
            end
        end
        out_code = out_code || " end;"

        return out_code
