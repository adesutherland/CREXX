options levelb
namespace qualified_bif_direct_namespace
import rxfnsb

checker = .AbbrevChecker()
say checker.abbrev("quicker", "quick", 1)
return

AbbrevChecker: class
  *: factory
    return

  abbrev: method = .string
    arg string = .string, astring = .string, len = .int
    return .rxfnsb..abbrev(string, astring, len)
