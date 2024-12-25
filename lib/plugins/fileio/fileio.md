# File I/O Functions Documentation

## Table of Contents
1. [Quick Start Guide](#quick-start-guide)
2. [Basic File Operations](#basic-file-operations)
3. [Directory Operations](#directory-operations)
4. [File Management](#file-management)
5. [Error Handling](#error-handling)
6. [Implementation Details](#implementation-details)
7. [Platform-Specific Notes](#platform-specific-notes)
8. [Best Practices](#best-practices)
9. [Common Patterns](#common-patterns)
10. [Error Codes](#error-codes)
11. [Performance Optimization](#performance-optimization)
12. [Troubleshooting](#troubleshooting)

## Quick Start Guide

### Basic Usage
```rexx
/*Read file example*/  
myArray = .array~new
count = readall(myArray, "input.txt", 0)
/*Write file example*/
dataArray = .array~new  
dataArray[1] = "Hello World"
count = writeall(dataArray, "output.txt", 0)  
/*Check file existence*/
if exists("config.txt") then
say "File exists" 
```
## Basic File Operations

### READALL(array, filepath, maxlines)
```rexx
/*Read with error handling*/
array = .array~new
count = readall(array, "data.txt", 0)
select
when count >= 0 then
say count "lines read successfully"
when count = ERR_FILE_NOT_FOUND then
say "File not found"
when count = ERR_ACCESS_DENIED then
say "Access denied"
otherwise
say "Error reading file:" count
end
```

### WRITEALL(array, filepath, maxlines)
Writes lines from an array to a file.
- `array`: Array containing lines to write
- `filepath`: Path to the file to write
- `maxlines`: Maximum number of lines to write (0 for unlimited)
- Returns: Number of lines written or error code

Example:
```rexx
/*Write with error handling*/
rc = writeall(array, "output.txt", 0)
if rc < 0 then do
call logError "Write failed", rc
return rc
end
```

### APPENDALL(array, filepath, maxlines)
Appends lines from an array to an existing file.
- `array`: Array containing lines to append
- `filepath`: Path to the file to append to
- `maxlines`: Maximum number of lines to append (0 for unlimited)
- Returns: Number of lines appended or error code

Example:
```rexx
/* Append with size check */
size = filesize("app.log")
if size < 1000000 then do  /* Less than 1MB */
    rc = appendall(logArray, "app.log", 0)
end
else do
    /* Rotate log file */
    call move "app.log", "app.log.old"
    rc = writeall(logArray, "app.log", 0)
end
```

## Directory Operations

### READDIR(files, dirs, dirpath)
Lists contents of a directory, separating files and subdirectories.
- `files`: Output array for file names
- `dirs`: Output array for directory names
- `dirpath`: Path to the directory to read
- Returns: 0 on success or error code

Example:
```rexx
/* Recursive directory scanning */
procedure scanDirectory
    use arg path
    files = .array~new
    dirs = .array~new
    rc = readdir(files, dirs, path)
    
    /* Process files */
    do i = 1 to files~items
        say "File:" path"/"files[i]
    end
    
    /* Recurse into subdirectories */
    do i = 1 to dirs~items
        call scanDirectory path"/"dirs[i]
    end
return
```

## File Management

### EXISTS(filepath)
Checks if a file exists.
- `filepath`: Path to check
- Returns: 1 if exists, 0 if not

### FILESIZE(filepath)
Gets the size of a file in bytes.
- `filepath`: Path to the file
- Returns: File size in bytes or error code

### DELETE(filepath)
Deletes a file.
- `filepath`: Path to the file to delete
- Returns: ERR_SUCCESS or error code

### MOVE(oldpath, newpath)
Moves or renames a file.
- `oldpath`: Original file path
- `newpath`: New file path
- Returns: ERR_SUCCESS or error code

### COPYFILE(src, dest)
Copies a file from source to destination.
- `src`: Source file path
- `dest`: Destination file path
- Returns: ERR_SUCCESS or error code

### TRUNCATE(filepath, size)
Truncates or extends a file to specified size.
- `filepath`: Path to the file
- `size`: New size in bytes
- Returns: ERR_SUCCESS or error code

### CHMOD(filepath, mode)
Changes file permissions.
- `filepath`: Path to the file
- `mode`: New permission mode
- Returns: ERR_SUCCESS or error code

## Implementation Details

### Buffer Management
```
/*Default buffer sizes  
  BUFFER_SIZE = 8192  -  8KB default buffer  
  MAX_LINE_LENGTH = 32000 Maximum line length  
  MAX_LINES = 100000  Maximum lines per file  */

/* Buffer optimization example*/
procedure readLargeFile
use arg filepath
buffer = .array~new
totalLines = 0
do while totalLines < MAX_LINES
count = readall(buffer, filepath, 1000) /* Read in chunks */
if count <= 0 then leave
totalLines = totalLines + count
call processBuffer buffer
end
return totalLines
```

### Memory Management
```
/*Pre-allocate array for large files*/
procedure efficientRead
use arg filepath
size = filesize(filepath)
estimatedLines = size % 100 / Estimate lines based on average line length /
array = .array~new(estimatedLines)
return readall(array, filepath, 0)
```

### Safe File Update
```rexx
procedure safeUpdate
    use arg filepath, newData
    /* Create backup */
    backupPath = filepath || '.bak'
    rc = copyfile(filepath, backupPath)
    if rc \= 0 then return rc   
    /* Update file */
    rc = writeall(newData, filepath, 0)
    if rc \= 0 then do
        /* Restore from backup */
        call delete filepath
        return move(backupPath, filepath)
    end
    /* Clean up backup */
    return delete(backupPath)
```

## Platform-Specific Notes

### Windows
- Uses Win32 API for optimal performance
- Supports long paths (>260 chars) when enabled
- Handles network paths (UNC)
- NTFS-specific features supported

### POSIX (Linux/Unix/macOS)
- Native system calls used
- Full permission support
- Symbolic link handling
- Extended attributes where supported

### Common Considerations
- Use forward slashes (/) for paths
- Binary vs text mode differences
- File locking behavior variations
- Permission mapping between systems

### Batch Operations
```rexx:md
/*Batch file processing*/
procedure batchProcess
use arg fileList
results = .array~new
do i = 1 to fileList~items
filepath = fileList[i]
/*Read and process in one pass*/
array = .array~new
rc = readall(array, filepath, 0)
if rc >= 0 then
results[i] = processData(array)
end
return results
```
## Error Codes
- `ERR_SUCCESS` (0): Operation successful
- `ERR_FILE_NOT_FOUND` (-1): File not found
- `ERR_ACCESS_DENIED` (-2): Access denied
- `ERR_PATH_TOO_LONG` (-3): Path exceeds maximum length
- `ERR_INVALID_HANDLE` (-14): Invalid file handle
- `ERR_READ_ERROR` (-15): Error reading file
- `ERR_WRITE_ERROR` (-16): Error writing file
- `ERR_BUFFER_FULL` (-17): Buffer overflow
- `ERR_NULL_POINTER` (-18): Null pointer provided
- `ERR_SEEK_ERROR` (-19): File seek error
- `ERR_PERMISSION` (-20): Permission denied
- `ERR_FILE_LOCKED` (-21): File is locked
- `ERR_FILE_TOO_LARGE` (-22): File too large
- `ERR_INVALID_MODE` (-23): Invalid file mode
- `ERR_FILE_EXISTS` (-24): File already exists
- `ERR_DIR_NOT_EMPTY` (-25): Directory not empty
- `ERR_FILE_BUSY` (-26): File is busy
- `ERR_DISK_FULL` (-27): Disk is full
- `ERR_FILE_CORRUPT` (-28): File is corrupt
- `ERR_TEMP_UNAVAIL` (-29): Temporary unavailable

## Version History

### Version 1.1
- Added enhanced error handling
- Improved cross-platform compatibility
- Added file rotation support
- Enhanced directory management

### Version 1.0
- Initial release
- Basic file operations
- Directory operations
- Error handling