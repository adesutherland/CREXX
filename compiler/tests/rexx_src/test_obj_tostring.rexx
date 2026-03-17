options levelb

a = .myclass()
say a

return

myclass: class
    *: factory
        return
        
    tostring: method = .string
        return "myclass instance"