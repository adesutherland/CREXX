options levelb
namespace set2_lib expose Person

Person: class
  name = .string
  age = .int

  *: factory
    arg n = .string, a = .int
    name = n
    age = a
    return

  get_name: method = .string
    return name

  get_age: method = .int
    return age

  say_hello: method
    say "Hello, my name is" name "and I am" age "years old."
    return
