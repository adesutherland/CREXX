/* GetEnv */
options levelb

namespace rxfnsb expose getenv

getenv: procedure = .string
    arg env_name = .string

    value = ""
    assembler getenv value,env_name
    return value
