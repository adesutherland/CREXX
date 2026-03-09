options levelb

var = .myclass(42)
say var.get_val()

return

myclass: class
    val = .int with register.1.int

    *: factory
        arg initial_value = .int
        val = initial_value
        return

    get_val: method = .int
        return val
