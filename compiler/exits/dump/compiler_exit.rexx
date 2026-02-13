options levelb
namespace rxcp expose exit_dispatch

/* Define Token Class to map the VM Objects passed by the Bridge */
token: class
    val_type = .int    with register.1.int
    val_text = .string with register.3.string

    *: factory
        return

    get_type: method = .int
        return val_type

    get_text: method = .string
        return val_text

/* The Bridge calls this procedure */
exit_dispatch: procedure = .string
    arg tokens = .token[]

    /* Sanity check: ensure we have tokens */
    if tokens.0 < 1 then return ""

    /* Check command trigger */
    cmd = tokens[1].get_text()
    if cmd \= "dump" then return ""

    /* Generate Code */
    /* DO block ensures a single instruction list container */
    out_code = "options levelb; do; "
    do i = 2 to tokens.0
        t_text = tokens[i].get_text()
        if t_text \= "" then do
            out_code = out_code || "say '" || t_text || "=' || " || t_text || ";"
        end
    end
    out_code = out_code || " end;"

    return out_code
