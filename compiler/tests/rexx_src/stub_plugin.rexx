options levelb
namespace stub expose stub_plugin

stub_plugin: procedure = .string
    arg tokens = .object
    say "Plugin (REXX) started"

    /* tokens is an object with attributes as elements */
    obj = .object
    assembler linkattr1 obj, tokens, 1
    
    val_text = ""
    assembler linkattr1 val_text, obj, 3
    say "Token 1 text:" val_text
    
    val_type = 0
    assembler linkattr1 val_type, obj, 1
    say "Token 1 type:" val_type

    return "SAY 'HELLO FROM BRIDGE'"

/* The Token class used for mapping */
token: class
    val_type = .int with register.1.int
    val_text = .string with register.3.string

    *: factory
        arg t_val = .int, txt_val = .string
        val_type = t_val
        val_text = txt_val
        return

    get_type: method = .int
        return val_type

    get_text: method = .string
        return val_text
