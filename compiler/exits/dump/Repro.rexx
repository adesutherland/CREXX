options levelb
namespace rxcp expose Repro

Repro: class
    _status = .string with register.1

    *: factory
        _status = "INITIAL"

    process: method = .string
        _status = "MODIFIED"
        return "OK"

    get_status: method = .string
        return _status
