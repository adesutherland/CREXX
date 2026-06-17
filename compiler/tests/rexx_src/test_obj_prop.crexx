options levelb

obj = .myclass()

obj.bar = "10"
a = obj.bar
say a

if a \= "10" then return 1

obj["foo"] = "20"
b = obj["foo"]
say b

if b \= "20" then return 1

return 0

myclass: class
    val = .string

    *: factory
        return

    get: method = .string
        arg k = .string
        return val

    set: method
        arg k = .string, new_val = .string
        val = new_val
        return
