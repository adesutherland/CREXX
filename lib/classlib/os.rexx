options levelb

namespace system_os expose os
import system

os: class
val = .int

  *: factory
    return

    /** 
    * method uptime returns the system uptime in seconds
    */
  uptime: method = .string
    return sysuptime()
    
    /** 
    * method user returns the logged on userid
    */
  user: method = .string
    return userid()  
    
