options levelb comments_dash
namespace system_os expose os
import system

/**
* class os is comprised of methods for which interaction with
* the operating system is necessary. It is is backed by the
* system plugin which does so in a platform independent way. 
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
    * @return .string loadpath
    */
  loadpath: method = .string
    return getLoadPath()
    
    /**
    * Returns the operating system name.
    *
    * @return Operating system name
    */
  osname: method = .string
    return opsys()
    
    /**
    * Appends one binary file to another.
    *
    * @param source Source file
    * @param target Target file
    * @return       1 on success, 0 on failure
    */
  appendBinaryFile: method = .int
    arg source = .string, target = .string
    return append(source, target)

    /**
    * Returns the host or computer name.
    *
    * @return Host name
    */
  getHost: method = .string
    return host()
    
    /**
    * Emits a beep.
    *
    * @return 1 on success, 0 on failure
    */
  beep: method = .int
    say 'TODO: beep is not fully implemented'
    return _beep()

    /**
   * Waits for a number of milliseconds.
   *
   * @param time 
   * @return     1 on success, 0 on failure
   */
  wait: method = .int
    arg time = .int
    return _wait(time)

    /**
    * Lists modules in an rxbin source.
    *
    * @param source rxbin file
    * @return       Status code or count, depending on native implementation
    */
  rxbinModules: method = .int
    arg source = .string
    return lmodules(source)

    /**
    * method listDir returns the files in a directory
    * @parm .string file directory name
    * @ return .string[]
    */
  listDir: method = .int
    arg file = .string, entries = .string[]
    say 'TODO: listdir needs to be tested'
    return _listdir(file,entries)

    /*
    * method getEnv
    * returns the contents of an environment variable
    * @parm .string name
    * @return .string content
    * note: this duplicates the getenv() bif.
    */
  getEnv: method = .string
    arg env_name = .string
    value = ""
    assembler getenv value,env_name
    return value

