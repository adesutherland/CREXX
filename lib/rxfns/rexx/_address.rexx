/* REXX LEVEB B ADDRESS FUNCTIONS */
options levelb
namespace _rxsysb expose _address _noredir _redir2array _redir2string _array2redir _string2redir
# import rxfnsb

/* This is the function that the compiler calls for the ADDRESS instruction */
_address: procedure = .int
  arg env = "shell", command = "", in = .binary, out = .binary, err = .binary, expose ... = .string

  shell_flags = 0; /* Flags for the shell command - see spawn instruction for values */
  rc = 0

  if env = "shell" then do
    shell_flags = 0
  end
  else if env = "crexxsaa" then do
    shell_flags = 1
  end
  else do
    return 1
  end

  redirect = .binary[5] /* Array to hold the redirect information for the rxas spawn instruction */
  redirect[1] = in /* Set up redirect[1] to hold the input stream */
  redirect[2] = out /* Set up redirect[2] to hold the output stream */
  redirect[3] = err /* Set up redirect[3] to hold the error stream */

  /* Set up redirect[4] to hold the environment variables                    */
  /* After this redirect[4] is an array of "pointers" to the args            */
  /* We need to do this with assembler as cREXX does not support objects yet */
  /* Note that expose is an array of names and arguments interlaced as setup */
  /* by the compiler. The first of each pair is the name of the variable in  */
  /* the string attribute and the type code in the integer attribute. The    */
  /* second in each pair is the register/variable itself. The compiler       */
  /* will have created string representation of the variable.                */
  /* This function just needs to pass all these to the spawn assembler       */
  /* instruction.                                                            */
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

  /* Set up redirect[5] to hold spawn flags as an integer - again using assembler at this time */
  assembler linktoattr1 5,redirect,shell_flags /* Link shell_flags to attribute 5 (1 base) of redirect */

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
