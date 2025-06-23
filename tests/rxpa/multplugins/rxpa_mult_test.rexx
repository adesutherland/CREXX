/* rxpa (plugin architecture) test */
/* Tests multiple plugins being used at the same time */

options levelb
import multplugin1
import multplugin2

say "Multiple plugin test"

say plugin1_echo("Hello from plugin1")
say plugin2_echo("Hello from plugin2")

return 0
