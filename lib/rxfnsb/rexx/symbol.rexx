/* REXX Levelb SYMBOL Implementation */
options levelb

namespace rxfnsb expose symbol

symbol: procedure = .string
    arg inputsymbol = .string
    if inputsymbol = "" then return "BAD"

    result = ""
    reg = strip(lower(inputsymbol))
    sym = ""
    type = ""
    module = 0
    addr = 0
    meta_array = 0
    meta_entry = ""

    /* First check if it is a valid symbol */
    /* Characters must be a letter, number, full stop or underscore */
    /* Note this included 0.23.4 as a valid symbol ... should we change this for level b? */
    do i = 1 to length(inputsymbol)
        c = substr(inputsymbol, i, 1)
        if pos(c, 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.') = 0 then return 'BAD'
    end

    /* Check if it is a reserved word - level b */
    keywords = "address arg assembler by call digits do drop else end error exit expose for forever if import input interpret",
               "iterate leave loop namespace nop numeric off on options otherwise output parse procedure pull push queue return",
               "say select signal then to trace until upper value var version void when while with "
    if pos(lower(inputsymbol) || " ", keywords) > 0 then return 'BAD'

    /* Look for the symbol in the metadata */
    address_object = 0
    assembler metaloadcalleraddr address_object /* Address from where are we called */
    assembler linkattr1 module,address_object,1  /* 1 = Module number */
    assembler linkattr1 addr,address_object,2 /* 2 = Address in module */

    /* Read the addresses backwards */
    result = "LIT" /* Assume a unused variable name */
    do a = addr to 0 by -1
        /* Get the metadata for that address */
        assembler metaloaddata meta_array,module,a
        do i = 1 to meta_array
            assembler linkattr1 meta_entry,meta_array,i
            if meta_entry = ".meta_clear" then do /* Object type */
                assembler linkattr1 sym,meta_entry,1
                if pos("."reg"@",sym"@") > 0 then do /* Rough and ready find */
                    /* result = "LIT" */ /* We have found a unused variable name */
                    leave a /* Leave the address loop - we have found the previous procedure in the module */
                end
            end

            else if meta_entry = ".meta_const" then do /* Object type */
                assembler linkattr1 sym,meta_entry,1
                if pos("."reg"@",sym"@") > 0 then do /* Rough and ready find */
                    result = "VAR" /* We have found a constant - but in this context it is a variable */
                    leave a
                end
            end

            else if meta_entry = ".meta_reg" then do /* Object type */
                assembler linkattr1 sym,meta_entry,1
                if pos("."reg"@",sym"@") > 0 then do /* Rough and ready find */
                    result = "VAR" /* We have found a variable */
                    leave a
                end
            end
        end
    end

    return result

