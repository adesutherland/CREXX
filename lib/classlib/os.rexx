options levelb
namespace system_os expose os
import system

/**
 * class os is comprised of methods for which interaction with
 * the operating system is necessary. It does so in a platform 
 * independent way and is backed by the system plugin.
 */
os: class

/** 
 * the factory method returns an instance of the os class
 * @ return .os
 */
  *: factory
    return

    /** 
    * method uptime returns the system uptime in seconds
    * @return .string uptime
    */
  uptime: method = .string
    return sysuptime()
    
    /** 
    * method user returns the logged on userid
    * @return .string userid tsk
    */
  user: method = .string
    return userid()  

    /** 
    * method getLoadPath returns the path from which
    * the program is run
    * @return .string userid tsk
    */
  loadpath: method = .string
    return getLoadPath()
    
