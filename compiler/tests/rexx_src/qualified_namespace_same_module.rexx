options levelb
namespace qnslocal

widget: class
  _name = .string

  *: factory
    arg name = .string
    _name = name
    return

  describe: method = .string
    return _name

build: procedure = .string
  local = .qnslocal..widget
  local = .qnslocal..widget("local")
  return local.describe()

main: procedure
  direct = .qnslocal..widget
  direct = .qnslocal..widget("direct")
  say qnslocal..build()
  say direct.describe()
  return
