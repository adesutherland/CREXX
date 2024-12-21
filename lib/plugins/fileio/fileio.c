//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file

#ifdef _WIN32

#include "windows.h"

#endif

#include <fcntl.h>
#include <sys/stat.h>

#ifdef _WIN32

#include <io.h>

#else
#include <unistd.h>
#endif

#ifdef _WIN32
#define MAX_PATH_LENGTH 260  // Windows MAX_PATH
#else
#define MAX_PATH_LENGTH 4096 // Common max path length for POSIX systems
#endif

// Add more error codes
#define ERR_SUCCESS         0
#define ERR_FILE_NOT_FOUND -1
#define ERR_ACCESS_DENIED  -2
#define ERR_PATH_TOO_LONG  -3
#define ERR_INVALID_HANDLE -14
#define ERR_READ_ERROR    -15
#define ERR_WRITE_ERROR   -16
#define ERR_BUFFER_FULL   -17
#define ERR_NULL_POINTER  -18
#define ERR_SEEK_ERROR    -19
#define ERR_PERMISSION    -20
#define ERR_FILE_LOCKED    -21
#define ERR_FILE_TOO_LARGE -22
#define ERR_INVALID_MODE   -23
#define ERR_FILE_EXISTS    -24
#define ERR_DIR_NOT_EMPTY  -25
#define ERR_FILE_BUSY      -26
#define ERR_DISK_FULL      -27
#define ERR_FILE_CORRUPT   -28
#define ERR_TEMP_UNAVAIL   -29

// Add file mode constants
#define FILE_MODE_READ    0
#define FILE_MODE_WRITE   1
#define FILE_MODE_APPEND  2

#define RETURN_ERROR(code) do { RETURNINT(code); PROCRETURN } while(0)

// Macro wrapper for the check path function
#define CHECK_PATH(path) do { \
    int result = checkPath(path); \
    if (result != ERR_SUCCESS) { \
        RETURN_ERROR(result); \
    } \
} while(0)

// Add helper function for safe string operations
int safeStringCopy(char *dest, size_t destSize, const char *src) {
    if (!dest || !src || destSize == 0) return ERR_NULL_POINTER;
    size_t srcLen = strlen(src);
    if (srcLen >= destSize) return ERR_BUFFER_FULL;
    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
    return ERR_SUCCESS;
}

void searchReplace(char *str, char search, char replace) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {  // Loop until the end of the string
        if (str[i] == search) {
            str[i] = replace;  // Replace the character
        }
    }
}

// Helper function to validate path
static int checkPath(const char* path) {
    if (!path) return ERR_NULL_POINTER;
    if (strlen(path) >= MAX_PATH_LENGTH) return ERR_PATH_TOO_LONG;

    // Check if path exists
    struct stat st;
    if (stat(path, &st) != 0) return ERR_FILE_NOT_FOUND;

    // Check if it's a directory
    if (!S_ISDIR(st.st_mode)) return ERR_FILE_NOT_FOUND;

    return ERR_SUCCESS;
}


// ... rest of the code ...

// Add helper function for file mode validation
static int validateFileMode(int mode) {
    return (mode >= FILE_MODE_READ && mode <= FILE_MODE_APPEND);
}

// Add helper function for file existence with mode check
static int checkFileAccess(const char *path, int mode) {
    if (!path) return ERR_NULL_POINTER;

    FILE *file;
    switch (mode) {
        case FILE_MODE_READ:
            file = fopen(path, "r");
            break;
        case FILE_MODE_WRITE:
        case FILE_MODE_APPEND:
            file = fopen(path, "a");
            break;
        default:
            return ERR_INVALID_MODE;
    }

    if (file) {
        fclose(file);
        return ERR_SUCCESS;
    }
    return ERR_ACCESS_DENIED;
}

// Add file locking helper functions
static int lockFile(FILE *file) {
#ifdef _WIN32
    HANDLE hFile = (HANDLE) _get_osfhandle(_fileno(file));
    if (hFile == INVALID_HANDLE_VALUE) return ERR_INVALID_HANDLE;
    if (!LockFile(hFile, 0, 0, MAXDWORD, MAXDWORD)) return ERR_FILE_LOCKED;
#else
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    if (fcntl(fileno(file), F_SETLK, &lock) == -1) return ERR_FILE_LOCKED;
#endif
    return ERR_SUCCESS;
}

static int unlockFile(FILE *file) {
#ifdef _WIN32
    HANDLE hFile = (HANDLE) _get_osfhandle(_fileno(file));
    if (hFile == INVALID_HANDLE_VALUE) return ERR_INVALID_HANDLE;
    if (!UnlockFile(hFile, 0, 0, MAXDWORD, MAXDWORD)) return ERR_FILE_LOCKED;
#else
    struct flock lock;
    lock.l_type = F_UNLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    if (fcntl(fileno(file), F_SETLK, &lock) == -1) return ERR_FILE_LOCKED;
#endif
    return ERR_SUCCESS;
}

PROCEDURE(readall) {
    int lines = 1, llen, amax = 0, amaxl, maxlines;
    searchReplace(GETSTRING(ARG1), '\\', '/');
    maxlines = GETINT(ARG2);
    FILE *file = fopen(GETSTRING(ARG1), "r");
    if (file == NULL) {
        RETURNINT(-8);
        PROCRETURN
    }
    char line[32000];      // 32k should be enough
    amaxl = 250;             // define new junk to allow additional lines in exceeded array
    while (fgets(line, sizeof(line), file) != NULL) {
        llen = strlen(line);
        if (line[llen - 1] == '\n') line[llen - 1] = '\0';
        if (amax == 0) {
            amax = amaxl;
            SETARRAYHI(ARG0, GETARRAYHI(ARG0) + amaxl);
        }
        SETSARRAY(ARG0, lines - 1, line);   // push into array

        if (maxlines > 0 && lines >= maxlines) break;
        lines++;
        amax--;
    }
    SETARRAYHI(ARG0, lines);  // adjust arrayhi
    RETURNINT(lines);
    PROCRETURN
    ENDPROC
}

PROCEDURE(writeall) {
    int lines = 0, tto, i, maxlines;
    searchReplace(GETSTRING(ARG1), '\\', '/');
    FILE *file = fopen(GETSTRING(ARG1), "w");
    if (file == NULL) {
        RETURNINT(-8);
        PROCRETURN
    }
    tto = GETARRAYHI(ARG0);
    maxlines = GETINT(ARG2);
    for (i = 0; i < tto; ++i) {
        if (lines > 0) fputs("\n", file);   // complete last record, avoids NL for last record
        fputs(GETSARRAY(ARG0, i), file);
        lines++;
        if (maxlines > 0 && lines >= maxlines) break;
    }
    fclose(file);
    RETURNINT(lines);
    PROCRETURN
    ENDPROC
}

PROCEDURE(readdir) {
    const size_t PATH_SEPARATOR_LEN = 1;  // Length of "/" or "\"
    int dirCount = 0;
    int fileCount = 0;
    char fullPath[MAX_PATH_LENGTH];
    size_t basePathLen;

    CHECK_PATH(GETSTRING(ARG2));

    searchReplace(GETSTRING(ARG2), '\\', '/');
    basePathLen = strlen(GETSTRING(ARG2));

    // Ensure path ends with separator but don't overflow
    if (basePathLen + PATH_SEPARATOR_LEN >= MAX_PATH_LENGTH) {
        RETURN_ERROR(ERR_PATH_TOO_LONG);
    }

    strncpy(fullPath, GETSTRING(ARG2), MAX_PATH_LENGTH - 1);
    if (fullPath[basePathLen - 1] != '/') {
        fullPath[basePathLen] = '/';
        fullPath[basePathLen + 1] = '\0';
        basePathLen++;
    }

#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    char searchPath[MAX_PATH_LENGTH];

    if (basePathLen + 1 >= MAX_PATH_LENGTH) {
        RETURN_ERROR(ERR_PATH_TOO_LONG);
    }
    snprintf(searchPath, sizeof(searchPath), "%s*", fullPath);

    HANDLE hFind = FindFirstFile(searchPath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        RETURN_ERROR(ERR_FILE_NOT_FOUND);
    }

    do {
        if (findFileData.cFileName[0] == '.') continue;

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            SETARRAYHI(ARG0, dirCount + 1);
            SETSARRAY(ARG0, dirCount++, findFileData.cFileName);
        } else {
            SETARRAYHI(ARG1, fileCount + 1);
            SETSARRAY(ARG1, fileCount++, findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
#else
    // Add POSIX support
    DIR* dir = opendir(fullPath);
    struct dirent* entry;
    
    if (!dir) {
        RETURN_ERROR(ERR_FILE_NOT_FOUND);
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        // Construct full path for stat
        if (basePathLen + strlen(entry->d_name) + 1 >= MAX_PATH_LENGTH) {
            closedir(dir);
            RETURN_ERROR(ERR_PATH_TOO_LONG);
        }
        
        snprintf(fullPath + basePathLen, MAX_PATH_LENGTH - basePathLen, "%s", entry->d_name);
        
        struct stat statbuf;
        if (stat(fullPath, &statbuf) == 0) {
            if (S_ISDIR(statbuf.st_mode)) {
                SETARRAYHI(ARG0, dirCount + 1);
                SETSARRAY(ARG0, dirCount++, entry->d_name);
            } else {
                SETARRAYHI(ARG1, fileCount + 1);
                SETSARRAY(ARG1, fileCount++, entry->d_name);
            }
        }
    }
    
    closedir(dir);
#endif
    RETURNINT(0);
    PROCRETURN
    ENDPROC
}

// Add new procedure for file append
PROCEDURE(appendall) {
    int lines = 0, tto, i, maxlines;
    searchReplace(GETSTRING(ARG1), '\\', '/');
    FILE *file = fopen(GETSTRING(ARG1), "a");
    if (file == NULL) {
        RETURNINT(ERR_FILE_NOT_FOUND);
        PROCRETURN
    }

    tto = GETARRAYHI(ARG0);
    maxlines = GETINT(ARG2);

    fseek(file, -1, SEEK_END); // Gehe zum Ende der Datei
    int lastChar = fgetc(file);
    if (lastChar != '\n') {
        fputs("\n", file); // Neue Zeile hinzufügen, wenn keine vorhanden ist
    }

    for (i = 0; i < tto; ++i) {
        if (fputs(GETSARRAY(ARG0, i), file) == EOF) {
            fclose(file);
            RETURNINT(ERR_WRITE_ERROR);
            PROCRETURN
        }
        fputs("\n", file);
        lines++;
        if (maxlines > 0 && lines >= maxlines) break;
    }
    fclose(file);
    RETURNINT(lines);
    PROCRETURN
    ENDPROC
}
// Add new procedure for file copy
PROCEDURE(copyfile) {
    const char *src = GETSTRING(ARG0);
    const char *dest = GETSTRING(ARG1);
    char buffer[8192];
    size_t bytesRead;

    FILE *source = fopen(src, "rb");
    if (!source) {
        RETURNINT(ERR_FILE_NOT_FOUND);
        PROCRETURN
    }

    FILE *target = fopen(dest, "wb");
    if (!target) {
        fclose(source);
        RETURNINT(ERR_ACCESS_DENIED);
        PROCRETURN
    }

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytesRead, target) != bytesRead) {
            fclose(source);
            fclose(target);
            RETURNINT(ERR_WRITE_ERROR);
            PROCRETURN
        }
    }

    fclose(source);
    fclose(target);
    RETURNINT(ERR_SUCCESS);
    PROCRETURN
    ENDPROC
}
// Add new procedure for file existence check
PROCEDURE(exists) {
    const char *path = GETSTRING(ARG0);
    FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        RETURNINT(1);
    }
    RETURNINT(0);
    PROCRETURN
    ENDPROC
}
// Add new procedure for file size
PROCEDURE(filesize) {
    const char *path = GETSTRING(ARG0);
    FILE *file = fopen(path, "rb");
    if (!file) {
        RETURNINT(ERR_FILE_NOT_FOUND);
        PROCRETURN
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        RETURNINT(ERR_SEEK_ERROR);
        PROCRETURN
    }

    long size = ftell(file);
    fclose(file);
    RETURNINT(size);
    PROCRETURN
    ENDPROC
}
// Add new procedure for file deletion
PROCEDURE(deletefile) {
    const char *path = GETSTRING(ARG0);
    if (remove(path) != 0) {
        RETURNINT(ERR_ACCESS_DENIED);
        PROCRETURN
    }
    RETURNINT(ERR_SUCCESS);
    PROCRETURN
    ENDPROC
}
// Add new procedure for file rename/move
PROCEDURE(movefile) {
    const char *oldpath = GETSTRING(ARG0);
    const char *newpath = GETSTRING(ARG1);

    if (rename(oldpath, newpath) != 0) {
        RETURNINT(ERR_ACCESS_DENIED);
        PROCRETURN
    }
    RETURNINT(ERR_SUCCESS);
    PROCRETURN
    ENDPROC
}
// Add new procedure for file truncation
PROCEDURE(truncatex) {
    const char *path = GETSTRING(ARG0);
    long size = GETINT(ARG1);

    FILE *file = fopen(path, "r+");
    if (!file) {
        RETURNINT(ERR_FILE_NOT_FOUND);
        PROCRETURN
    }

    int lock_result = lockFile(file);
    if (lock_result != ERR_SUCCESS) {
        fclose(file);
        RETURNINT(lock_result);
        PROCRETURN
    }

#ifdef _WIN32
    if (_chsize(_fileno(file), size) != 0) {
#else
        if (ftruncate(fileno(file), size) != 0) {
#endif
        unlockFile(file);
        fclose(file);
        RETURNINT(ERR_WRITE_ERROR);
        PROCRETURN
    }

    unlockFile(file);
    fclose(file);
    RETURNINT(ERR_SUCCESS);
    PROCRETURN
    ENDPROC
}
// Add new procedure for file permissions
PROCEDURE(chmodx) {
    const char *path = GETSTRING(ARG0);
    int mode = GETINT(ARG1);

#ifdef _WIN32
    // Windows implementation using SetFileAttributes
    DWORD attributes = FILE_ATTRIBUTE_NORMAL;
    if (!(mode & 0200)) attributes |= FILE_ATTRIBUTE_READONLY;
    if (!SetFileAttributes(path, attributes)) {
        RETURNINT(ERR_PERMISSION);
        PROCRETURN
    }
#else
        if (chmod(path, mode) != 0) {
RETURNINT(ERR_PERMISSION);
PROCRETURN
}
#endif

    RETURNINT(ERR_SUCCESS);
    PROCRETURN
    ENDPROC
}
// File IO function definitions
LOADFUNCS
    ADDPROC(readall, "fileio.readall", "b", ".int",
            "expose array=.string[],expose file=.string,arg2=.int");
    ADDPROC(writeall, "fileio.writeall", "b", ".int",
            "expose array=.string[],file=.string,arg2=.int");
    ADDPROC(readdir, "fileio.readdir", "b", ".int",
            "expose entries=.string[],expose dirs=.string[],file=.string");
    ADDPROC(appendall, "fileio.appendall", "b", ".int",
            "expose array=.string[],file=.string,arg2=.int");
    ADDPROC(copyfile, "fileio.copyfile", "b", ".int",
            "expose src=.string,expose dest=.string");
    ADDPROC(exists, "fileio.exists", "b", ".int", "expose path=.string");
    ADDPROC(filesize, "fileio.filesize", "b", ".int", "expose path=.string");
    ADDPROC(deletefile, "fileio.delete", "b", ".int", "expose path=.string");
    ADDPROC(movefile, "fileio.move", "b", ".int","expose oldpath=.string,expose newpath=.string");
    ADDPROC(truncatex, "fileio.truncate", "b", ".int", "expose path=.string,size=.int");
    ADDPROC(chmodx, "fileio.chmod", "b", ".int", "expose path=.string,mode=.int");
ENDLOADFUNCS
/*
# File I/O Functions Documentation

## Basic File Operations

### fileio.readall(array, filepath, maxlines)
Reads lines from a file into an array.
- `array`: Output array to store the lines
- `filepath`: Path to the file to read
- `maxlines`: Maximum number of lines to read (0 for unlimited)
- Returns: Number of lines read or error code

### fileio.writeall(array, filepath, maxlines)
Writes lines from an array to a file.
- `array`: Array containing lines to write
- `filepath`: Path to the file to write
- `maxlines`: Maximum number of lines to write (0 for unlimited)
- Returns: Number of lines written or error code

### fileio.appendall(array, filepath, maxlines)
Appends lines from an array to an existing file.
- `array`: Array containing lines to append
- `filepath`: Path to the file to append to
- `maxlines`: Maximum number of lines to append (0 for unlimited)
- Returns: Number of lines appended or error code

## Directory Operations

### fileio.readdir(files, dirs, dirpath)
Lists contents of a directory, separating files and subdirectories.
- `files`: Output array for file names
- `dirs`: Output array for directory names
- `dirpath`: Path to the directory to read
- Returns: 0 on success or error code

## File Management

### fileio.exists(filepath)
Checks if a file exists.
- `filepath`: Path to check
- Returns: 1 if exists, 0 if not

### fileio.filesize(filepath)
Gets the size of a file in bytes.
- `filepath`: Path to the file
- Returns: File size in bytes or error code

### fileio.delete(filepath)
Deletes a file.
- `filepath`: Path to the file to delete
- Returns: ERR_SUCCESS or error code

### fileio.move(oldpath, newpath)
Moves or renames a file.
- `oldpath`: Original file path
- `newpath`: New file path
- Returns: ERR_SUCCESS or error code

### fileio.copyfile(src, dest)
Copies a file from source to destination.
- `src`: Source file path
- `dest`: Destination file path
- Returns: ERR_SUCCESS or error code

### fileio.truncate(filepath, size)
Truncates or extends a file to specified size.
- `filepath`: Path to the file
- `size`: New size in bytes
- Returns: ERR_SUCCESS or error code

### fileio.chmod(filepath, mode)
Changes file permissions.
- `filepath`: Path to the file
- `mode`: New permission mode
- Returns: ERR_SUCCESS or error code

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
 */