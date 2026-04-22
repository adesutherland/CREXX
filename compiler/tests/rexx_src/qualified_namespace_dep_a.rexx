options levelb
namespace qnsa expose widget create describe

widget: class
  _name = .string

  *: factory
    arg name = .string
    _name = "A-" || name
    return

  describe: method = .string
    return _name

create: procedure = .qnsa..widget
  arg name = .string
  value = .qnsa..widget
  value = .widget(name)
  return value

describe: procedure = .string
  arg value = .qnsa..widget
  return value.describe()
