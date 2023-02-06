/* rexx */
options levelb

namespace _rxsysb expose raise

/* Raise() Internal Function to Raise a runtime error */
raise: procedure = .int
  arg type = .string, code = .string, parm1 = .string
  /* TODO Variable number of arguments */
  say "RUNTIME" type "ERROR:" code parm1
  /*
  exit /* TODO - the compiler does not supports exit yet */
  */
  return 0
