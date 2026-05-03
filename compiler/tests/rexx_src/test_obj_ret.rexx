options levelb

test_obj: class
    val_text = .string

    *: factory
        arg txt_val = .string
        val_text = txt_val
        return

    get_text: method = .string
        return val_text

main: procedure = .int
    x = "HelloWorld";
    t = .test_obj(x)
    say "Initial text:" t.get_text()
    
    /* Call it again. If it was stolen, it will be empty/null */
    text2 = t.get_text()
    say "Second call text:" text2
    
    if text2 = "HelloWorld" then say "SUCCESS: No stealing detected"
    else say "FAILURE: Attribute was stolen!"

    x = "changed"
    /* Call it again. Has the contents of the opbject been changed by changing the variable passed to the factory */
    text3 = t.get_text()
    say "Third call text:" text3

    if text3 = "HelloWorld" then say "SUCCESS: No factory change detected"
    else say "FAILURE: Factory mess up!"

    return 0
