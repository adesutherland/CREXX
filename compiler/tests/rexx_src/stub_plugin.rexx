options levelb
namespace stub expose stub_plugin token

/* The Token class used by the bridge test */
token: class
    val_type = .int
    val_text = .string

    /* Factory for the test */
    *: factory
        arg t=.int, st=.int, txt=.string, l=.int, c=.int, len=.int, f=.string, nt=.int, vt=.int
        val_type = t
        val_text = txt
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
