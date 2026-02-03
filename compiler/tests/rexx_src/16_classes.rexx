options levelb

var = .myclass(10)
say var.getvalue()

return;

myclass: class
    val = .int with register.1.int

    *: factory
        arg initial_value = .int
        val = initial_value
        return

    getvalue: method = .int
        return val

    setvalue: method
        arg new_val = .int
        val = new_val
        return
