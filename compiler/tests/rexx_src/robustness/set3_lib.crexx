options levelb
namespace set3_lib expose get_int_array get_person_array Item

Item: class
  val = .string
  *: factory
    arg v = .string
    val = v
    return
  get_val: method = .string
    return val

get_int_array: procedure = .int[]
  a = .int[3]
  a[1] = 10
  a[2] = 20
  a[3] = 30
  return a

get_person_array: procedure = .Item[]
  a = .Item[2]
  a[1] = .Item("First")
  a[2] = .Item("Second")
  return a
