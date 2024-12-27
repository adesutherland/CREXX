# System Functions Documentation
This documentation includes both system-level functions and file system operations.
Although most of them are designed to support multiple operating systems, they are currently activated only for Windows.
## GETENV
Retrieves the value of a system environment variable.

**Syntax:** `value = getenv(env_name)`

**Parameters:**
- `env_name` - String containing the environment variable name

**Returns:**
- `string` - Value of the environment variable if found
- `SIGNAL_FAILURE` - If variable not found or invalid argument

**Example:**
rexx
home = getenv("HOME")
path = getenv("PATH")

## GETDIR
Returns the current working directory path. Works cross-platform on Windows, Linux, and macOS.

**Syntax:** `path = getdir()`

**Parameters:**  
None

**Returns:**
- `string` - Current working directory path
- `SIGNAL_FAILURE` - If unable to get directory

**Example:**
rexx

## SETDIR
Changes the current working directory path. Works cross-platform on Windows, Linux, and macOS.

**Syntax:** `rc = setdir(new-current-working-directory)`

**Parameters:**  
None

**Returns:**
- 0  - Current working directory path has been changed
- -8 - unable to change the directory

## TESTDIR
Tests if a directory exists and is accessible. Path separators are automatically normalized.

**Syntax:** `rc = testdir(directory-to-check)`

**Parameters:**
- `directory` - String containing the directory path to test

**Returns:**
- `0` - Success: Directory exists
- `-8` - Error: Directory does not exist or is not accessible

**Example:**
```rexx
if testdir("/path/to/check") == 0 then say "Directory exists"
```

## CREATEDIR
Creates a new directory. On Unix systems, creates with permissions 0755.

**Syntax:** `rc = createdir(directory)`

**Parameters:**
- `directory` - String containing the directory path to create

**Returns:**
- `0` - Success: Directory created
- `-4` - Error: Directory already exists
- `-8` - Error: Unable to create directory

**Example:**
```rexx
rc = createdir("new_folder")
```

## RENAMEFILE
Renames or moves a file from the source path to the target path. Works across directories on the same filesystem.

**Syntax:** `rc = renamefile(oldname, newname)`

**Parameters:**
- `oldname` - String containing the source file path/name
- `newname` - String containing the target file path/name

**Returns:**
- `0` - Success: File was renamed
- `-4` - Error: Source file does not exist
- `-8` - Error: Rename operation failed (target exists, permission denied, etc.)

**Example:**
```rexx
rc = renamefile("old.txt", "new.txt")
```

## DELETEFILE
Deletes the specified file. Path separators are automatically normalized.

**Syntax:** `rc = deletefile(filename)`

**Parameters:**
- `filename` - String containing the file path to delete

**Returns:**
- `0` - Success: File deleted
- `-3` - Error: Permission denied
- `-4` - Error: File does not exist
- `-5` - Error: File is in use
- `-8` - Error: Other error

**Example:**
```rexx
rc = deletefile("unwanted.txt")
```
## SLEEP
Pauses execution for the specified duration in milliseconds.

**Syntax:** `rc = sleep(milliseconds)`

**Parameters:**
- `milliseconds` - Integer containing the time to sleep in milliseconds

**Returns:**
- `0` - Success: Sleep completed
- `-1` - Error: Invalid argument


## OPSYS
Returns the current operating system platform identifier.

**Syntax:** `platform = opsys()`

**Parameters:**  
None

**Returns:**
- `string`  - Platform identifier ("WINDOWS", "LINUX", "macOS", etc.)
- `unknown` - If unable to determine platform

**Example:**
```rexx
if opsys() == "WINDOWS" then say "Running on Windows"
```

## HOST
Returns the computer name of the current system.

**Syntax:** `name = host()`

**Parameters:**  
None

**Returns:**
- `string` - System hostname
- `unknown` - If unable to get hostname


**Example:**
```rexx
say "Computer name:" host()
```

## USERID
Returns the username of the current user.

**Syntax:** `name = userid()`

**Parameters:**  
None

**Returns:**
- `string`  - Current username
- `unknown` - If unable to get username

**Example:**
```rexx
say "Current user:" userid()