/* rxpa (plugin architecture) signal test */
/* This runs against the rxpa_dynlink and rxpa_staticlink plugins */

options levelb
import rxpatests

say "RXPA Signal Tests"

# Throw a signal
call throw_signal
say "Error - should not reach here"

return 1
