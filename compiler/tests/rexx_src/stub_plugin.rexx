options levelb
namespace stub expose stub_plugin

/* The Token class used for mapping */
token: class
    val_type = .int with register.1
    val_text = .string with register.3

    /* Factory only needed for internal testing */
    *: factory
        arg t_val = .int, txt_val = .string
        val_type = t_val
        val_text = txt_val
        return

    get_type: method = .int
        return val_type

    get_text: method = .string
        return val_text

stub_plugin: procedure = .string
    arg tokens = .token[] 
    say "Plugin (REXX) started"

    do i = 1 to tokens.0
        say "Token" i "type:" tokens[i].get_type() "text:" tokens.i.get_text()
    end

    return "SAY 'HELLO FROM BRIDGE'"