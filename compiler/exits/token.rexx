options levelb
namespace rxcp expose token

token: class
    val_type = .int    with register.1
    val_text = .string with register.3

    *: factory
        return

    get_type: method = .int
        return val_type

    get_text: method = .string
        return val_text
