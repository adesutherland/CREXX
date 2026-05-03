options levelb
import set5_lib

say "Calling public_func:" public_func()

/* Attempt to call a private function. This should fail at compile time. */
say "Calling private_func:" private_func()

return 0
