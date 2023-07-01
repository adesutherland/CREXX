/* REXX LEVEB B ADDRESS FUNCTIONS */
options levelb
namespace _rxsysb expose _address _noredir _redir2array _redir2string _array2redir _string2redir
# import rxfnsb

/* This is the function that the compiler calls for the ADDRESS instruction */
_address: procedure = .int
  arg env = "shell", command = "", in = .binary, out = .binary, err = .binary, expose ... = .string

  redirect = .binary[4]
  redirect[1] = in
  redirect[2] = out
  redirect[3] = err
  rc = 0

  /* Set up redirect[4] to hold the environment variables                    */
  /* After this redirect[4] is an array of "pointers" to the args            */
  /* We need to do this with assembler as cREXX does not support objects yet */
  num_envs = arg.0
  envs = .string[]
  e = .string
  assembler linkattr1 envs,redirect,4
  assembler setattrs envs,num_envs
  do i = 1 to num_envs
    assembler linkarg e,i,5         /* Link args[op2+op3] to op1 - there are 5 fixed arguments */
    assembler linktoattr1 i,envs,e  /* Link op3 to attribute op1 (1 base) of op2 */
    assembler unlink e
  end
  assembler unlink envs

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
