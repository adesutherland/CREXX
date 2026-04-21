options levelb
namespace interface_showcase_same_module

main: procedure
  selected = .asset("log.txt")
  alias = .namedasset
  alias = .fileasset("log.txt")

  sized = .asset.from_size(8)
  metric = .measured
  metric = .fileasset.from_size(8)

  fallback = .asset("memo")

  say selected.describe()
  say alias.name()
  say sized.describe()
  say metric.metric()
  say fallback.describe()
  return

asset: interface
  *: factory = .asset
  arg spec = .string
  from_size: factory = .asset
  arg size = .int

  describe: method = .string
    return kind() || ":" || name() || ":" || size()

  kind: method = .string
  name: method = .string
  size: method = .int

namedasset: interface
  name: method = .string

measured: interface
  metric: method = .int

fileasset: class implements .asset .namedasset .measured
  _name = .string
  _size = .int

  *: match
    arg spec = .string
    if spec = "log.txt" then return 100
    return 0

  *: factory
    arg spec = .string
    _name = spec
    _size = 8
    return

  from_size: match
    arg size = .int
    if size = 8 then return 50
    return 0

  from_size: factory
    arg size = .int
    _name = "sized-file"
    _size = size
    return

  kind: method = .string
    return "file"

  name: method = .string
    return _name

  size: method = .int
    return _size

  metric: method = .int
    return _size

cacheasset: class implements .asset
  _name = .string
  _size = .int

  *: factory
    arg spec = .string
    _name = spec
    _size = 1
    return

  from_size: factory
    arg size = .int
    _name = "cache-" || size
    _size = size
    return

  kind: method = .string
    return "cache"

  name: method = .string
    return _name

  size: method = .int
    return _size
