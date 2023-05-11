/* REXX LEVEB B ADDRESS FUNCTIONS */
options levelb
namespace _rxsysb expose _address _noredir _redir2array _redir2string _array2redir _string2redir
# import rxfnsb

/* This is the function that the compiler calls for the ADDRESS instruction */
_address: procedure = .int
  arg env = "shell", command = "", in = .binary, out = .binary, err = .binary

  redirect = .binary[1 to 3]
  redirect[1] = in
  redirect[2] = out
  redirect[3] = err
  rc = 0

  /* Spawn the process */
  assembler spawn rc,command,redirect
  return rc

/* Return a no redirect dummy Native Object */
_noredir: procedure = .binary
    handle = .binary
    return handle

/* Return a redirect to Array Native Object */
_redir2array: procedure = .binary
    arg expose arr = .string[]
    handle = .binary
    assembler redir2arr handle, arr
    return handle

/* Return a redirect to String Native Object */
_redir2string: procedure = .binary
    arg expose str = .string
    handle = .binary
    assembler redir2str handle, str
    return handle

/* Return a redirect from an Array Native Object */
_array2redir: procedure = .binary
    arg expose arr = .string[]
    handle = .binary
    assembler arr2redir handle, arr
    return handle

/* Return a redirect from a String Native Object */
_string2redir: procedure = .binary
    arg expose str = .string
    handle = .binary
    assembler str2redir handle, str
    return handle
