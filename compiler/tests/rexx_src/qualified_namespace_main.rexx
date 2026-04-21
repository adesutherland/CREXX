options levelb
namespace qualified_namespace_main
import qnsa
import qnsb

main: procedure
  left = .qnsa::widget
  right = .qnsb::widget

  left = .qnsa::widget("one")
  right = .qnsb::widget("two")

  say qnsa::describe(left)
  say qnsb::describe(right)
  say qnsa::describe(qnsa::create("three"))
  say qnsb::describe(qnsb::create("four"))
  return
