options levelb
namespace qnsb expose widget create describe

widget: class
  _name = .string

  *: factory
    arg name = .string
    _name = "B-" || name
    return

  describe: method = .string
    return _name

create: procedure = .qnsb..widget
  arg name = .string
  value = .qnsb..widget
  value = .widget(name)
  return value

describe: procedure = .string
  arg value = .qnsb..widget
  return value.describe()
