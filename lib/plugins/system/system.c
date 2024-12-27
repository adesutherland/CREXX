//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#if defined(__APPLE__)
 #include <sys/stat.h>
#include <unistd.h>        // For POSIX systems (Linux/macOS)
#define max(a,b)             \
  ({			     \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
  })

#define min(a,b)             \
  ({			     \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
  })
#endif
#ifdef _WIN32
  #include <direct.h>     // For Windows
  #include <windows.h>
  #define getcwd _getcwd  // Map to Windows-specific version
#else
  #include <dirent.h>
  #include <ctype.h>
#endif

#ifdef _WIN32
#define wait(ms) Sleep(ms);
#elif defined(__APPLE__)
#else
// #include <arpa/inet.h>    // Linux
   #define wait(ms) usleep(ms*1000)
#endif

#include "crexxpa.h"      // crexx/pa - Plugin Architecture header file
// distinguish between Windows and Linux and MAC
#if defined(_WIN32)
    #define REMOVE_DIR(path) _rmdir(path)
    #define MAKE_DIR(path)   _mkdir(path)
    #define TEST_DIR(path)   _access(path, 0)
    #define TEST_FILE(fname) _access(fname, 0)
    #define RENAME_FILE(source,target) MoveFileEx(source, target,0) //MOVEFILE_REPLACE_EXISTING);
#else
    #define REMOVE_DIR(path) rmdir(path)
    #define MAKE_DIR(path)   mkdir(path, 0755)
    #define TEST_DIR(path)   access(path, F_OK)
    #define TEST_FILE(fname) access(fname, F_OK)
    #define RENAME_FILE(source,target)  rename(source, target)
#endif

// replace \ chars by / (needed in Windows)
void searchReplace(char *str, char search, char replace) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {  // Loop until the end of the string
        if (str[i] == search) {
            str[i] = replace;  // Replace the character
        }
    }
}
/* -------------------------------------------------------------------------------------
 * Get environment variable (directory)
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(getEnv) {
    char *varName = GETSTRING(ARG0);    
    if (!varName) {
        RETURNSIGNAL(SIGNAL_FAILURE, "Invalid argument")
    }
    char *varValue = getenv(varName);   
    if (varValue == NULL) {             
        RETURNSIGNAL(SIGNAL_FAILURE, "Environment variable not found")
    }
    RETURNSTR(varValue);
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Get current working directory
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(getdir) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {  // works in Windows/Linux/Mac
        RETURNSTR(cwd);     /* Set the return value */
    } else {
        RETURNSIGNAL(SIGNAL_FAILURE, "Unable to get current working directory")
    }
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Set new current working directory
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(setdir) {
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (chdir(GETSTRING(ARG0)) == 0) {  // works in Windows/Linux/Mac
        RETURNINT(0);
    } else {
        RETURNINT(-8);
    }
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Test existence of directory
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(testdir) {
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (TEST_DIR(GETSTRING(ARG0)) == 0) {
        RETURNINT(0);
    } else {
        RETURNINT(-8);
    }
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Create new directory
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(createdir) {
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (TEST_DIR(GETSTRING(ARG0)) == 0) {
        RETURNINT(-4);     // already there
    } else {
        if (MAKE_DIR(GETSTRING(ARG0)) == 0) {
            RETURNINT(0);
        } else RETURNINT(-8);   // not created
    }
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Remove directory (must be empty)
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(removedir) {
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (TEST_DIR(GETSTRING(ARG0)) != 0)    RETURNINT(-4);             // does not exist
    else {
       if (REMOVE_DIR(GETSTRING(ARG0)) == 0) RETURNINT(0);
       else RETURNINT(-8);
    }
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Check existence of a file
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(testfile) {
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (TEST_FILE(GETSTRING(ARG0)) == 0) RETURNINT(0);
    else RETURNINT(-8);            // does not exist, or is not accessible
ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Remove file
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(deletefile) {
    char *filename = GETSTRING(ARG0);
    if (!filename) {
        RETURNINTX(-1);    // Invalid argument
    }
    searchReplace(filename, '\\', '/');
    
    if (TEST_FILE(filename) != 0) {
        RETURNINT(-4);    // File does not exist
    }
    
    if (remove(filename) != 0) {
        switch (errno) {
            case EACCES:
                RETURNINTX(-3);    // Permission denied
            case EBUSY:
                RETURNINTX(-5);    // File is in use
            default:
                RETURNINTX(-8);    // Other error
        }
    }
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(renamefileP) {
    int rc;
    searchReplace(GETSTRING(ARG0),'\\','/');
    searchReplace(GETSTRING(ARG1),'\\','/');
    if (TEST_FILE(GETSTRING(ARG0)) != 0)    RETURNINT(-4);             // does not exist
    else {
        rc=RENAME_FILE(GETSTRING(ARG0),GETSTRING(ARG1));
        if (rc==0) RETURNINT(0);
        else RETURNINT(-8);
    }
    ENDPROC
}
/* -------------------------------------------------------------------------------------
 * Get directory content
 * -------------------------------------------------------------------------------------
 */
PROCEDURE(listdir) {
    int indx=0;
    char vname[512];
    memset(vname, 0, sizeof(vname));
    searchReplace(GETSTRING(ARG0),'\\','/');
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    sprintf(vname, "%s%s",GETSTRING(ARG0),"/*");
    HANDLE hFind = FindFirstFile(vname, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        switch (GetLastError()) {
            case ERROR_PATH_NOT_FOUND:
                RETURNINTX(-4);    // Directory not found
            case ERROR_ACCESS_DENIED:
                RETURNINTX(-3);    // Permission denied
            default:
                RETURNINTX(-8);    // Other error
        }
        PROCRETURN;
    }
    SETARRAYHI(ARG1,0);
    do {
        if (findFileData.cFileName[0]=='.') continue;  // drop . and .. entries
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
   /* then */ sprintf(vname, "%s%s","> ",findFileData.cFileName);
        else  sprintf(vname, "%s%s","+ ",findFileData.cFileName);
        SETARRAYHI(ARG1,indx+1);
        SETSARRAY(ARG1,indx,vname);
        indx++;
    } while (FindNextFile(hFind, &findFileData) != 0);
    FindClose(hFind);
#else
    struct dirent *entry;
    DIR *dir = opendir(vname);
    if (dir == NULL) {
        switch (errno) {
            case ENOENT:
                RETURNINTX(-4);    // Directory not found
            case EACCES:
                RETURNINTX(-3);    // Permission denied
            default:
                RETURNINTX(-8);    // Other error
        }
        PROCRETURN;
    }
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            sprintf(vname, "%s%s","> ",entry->d_name);
        } else if (entry->d_type == DT_REG) {
            sprintf(vname, "%s%s","+ ",entry->d_name);
        } else {
            sprintf(vname, "%s%s","? ",entry->d_name);
        }
        SETARRAYHI(ARG1,indx+1);
        SETSARRAY(ARG1,indx,vname);
        indx++;
    }
    closedir(dir);
#endif
    RETURNINT(0);
    PROCRETURN
ENDPROC
}
//
// temporary
PROCEDURE(writeall) {
    int lines=0,tto,i,maxlines;
    searchReplace(GETSTRING(ARG1),'\\','/');
    FILE *file = fopen(GETSTRING(ARG1), "w");
    if (file == NULL) {
        RETURNINT(-8);
        PROCRETURN
    }
    tto=GETARRAYHI(ARG0);
    maxlines = GETINT(ARG2);
    for (i = 0 ; i < tto; ++i) {
        if (lines>0) fputs("\n",file);   // complete last record, avoids NL for last record
        fputs(GETSARRAY(ARG0,i),file);
        lines++;
        if (maxlines>0 && lines>=maxlines) break;
    }
    RETURNINT(lines);
    fclose(file);
    PROCRETURN
    ENDPROC
}

int nextdel(char * strg,int i, int plen,char pchar) {
    while (i < plen) {
      if (strg[i] == pchar) return i;
      i++;
    }
    return INT_MAX;
}
void trim(char *str) {
    char *start = str;  // Pointer auf den Anfang des Strings
    char *end;
    while (isspace((unsigned char)*start)) {   // skip leading blanks
        start++;
    }
    if (start != str) {            // shift chars to the beginning
       memmove(str, start, strlen(start) + 1);
    }
    end = str + strlen(str) - 1;   // set new end pointer
    while (end > str && isspace((unsigned char)*end)) { // skip trailing blanks
        *end = '\0'; // Nullify
        end--;
    }
}

PROCEDURE(parse) {
    int patternpos = 0, plen, indx=0;
    char * stringpos,* stringoffset;
    int begin, end;
    char tchar;
    char *string =  GETSTRING(ARG0);
    char *pattern = GETSTRING(ARG1);
    int debug = 0;
    char patstring[255];
    char valstring[512];
    char variable[64];
    plen = strlen(pattern);
    if (strstr(GETSTRING(ARG2), "debug") > 0) debug=1;

    RETURNINT(-8);
    stringoffset=string;
    while (patternpos < plen) {
        indx++;
     // locate first quote, can be a single or a double quote
        patternpos = min(nextdel(pattern, 0, plen, '"'), nextdel(pattern, patternpos, plen, '\"'));
        if (patternpos == INT_MAX) break;    // nothing there, 1. iteration to n. iteration
        tchar = pattern[patternpos];         // save the quote type
        begin = patternpos;                  // here the pattern begines
        patternpos = nextdel(pattern, patternpos + 1, plen, tchar);  // search the ending (same quote type)
        if (patternpos == INT_MAX) patternpos = plen;  // not found assume it lasts to the end of the pattern string
        end = patternpos;                    // save the end position
     // Extract string pattern and variable preceding it
     // ------------------------------------------------
        memset(patstring, 0, sizeof(patstring));
        memset(valstring, 0, sizeof(valstring));
        memset(variable,  0, sizeof(variable));

        strncpy(variable, pattern, begin);
        trim(variable);
        strncpy(patstring, pattern + begin, end - begin + 1);
        if (debug==1) printf("... located pattern : '%s'\n",patstring);
        if (strlen(variable)>0) {   // first pattern has no preceding variable
            if (debug==1)  printf("... located variable    : '%s'\n",variable);
            SETARRAYHI(ARG2, indx);
            SETSARRAY(ARG2, indx - 1, variable);
            SETARRAYHI(ARG3, indx);
            SETSARRAY(ARG3, indx - 1, "");
        } else printf("... no prior variable assignment\n");
        pattern = pattern + end + 1;
     // now find pattern in string
     // --------------------------
        stringpos = strstr(stringoffset, patstring);
        if (stringpos == 0) break;
        strncpy(valstring, stringoffset, stringpos - stringoffset);
        stringoffset = stringpos + strlen(patstring);
        trim(valstring);
        if (debug==1) printf("... extracted value : '%s' preceeding pattern\n",valstring);
        if (strlen(variable)>0) {   // first pattern has no preceding variable
             SETSARRAY(ARG3, indx - 1, valstring);
        } else indx--;   // there is no variable reset indx by -1
    }
 // now handle remaining parts past last pattern
    memset(valstring, 0, sizeof(valstring));
    memset(variable, 0, sizeof(variable));
    sprintf(valstring, "%s",stringoffset);
    trim(valstring);
    sprintf(variable, "%s",pattern);
    trim(variable);
    SETARRAYHI(ARG2, indx);
    SETARRAYHI(ARG3,indx);
    if (strlen(variable)==0) SETSARRAY(ARG2, indx - 1, "not_assigned");
       else SETSARRAY(ARG2, indx - 1, variable);
    SETSARRAY(ARG3,indx-1,valstring);
    RETURNINT(0);
    PROCRETURN
    ENDPROC
}

#if defined(_WIN32)
PROCEDURE(setclipboard) {
    char *text=GETSTRING(ARG0);

    if (!OpenClipboard(NULL)) RETURNINTX(-8);   // Open the clipboard
    EmptyClipboard();                       // Clear the clipboard

    size_t len = strlen(text) + 1;      // Allocate global memory for the text
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (!hMem) {
        CloseClipboard();
        RETURNINTX(-12);
    }
    memcpy(GlobalLock(hMem), text, len);  // Copy text into the memory
    GlobalUnlock(hMem);

    SetClipboardData(CF_TEXT, hMem);  // Set the clipboard data

    CloseClipboard();                         // Close the clipboard
    RETURNINTX(0);
    ENDPROC
}
PROCEDURE(getclipboard) {
    // Open the clipboard
    if (!OpenClipboard(NULL)) {
       RETURNSTRX("$$$ERROR$$$");
    }
    // Get clipboard data
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == NULL) {
        CloseClipboard();
        RETURNSTRX("");
    }
 // Lock the global memory and copy text from it
    char *text = (char *)GlobalLock(hData);
    if (text) {
        GlobalUnlock(hData);
        RETURNSTR(text);
    } else RETURNSTR("");
    // Close the clipboard
    CloseClipboard();
ENDPROC
}
#else
PROCEDURE(setclipboard) {
    #if defined(__APPLE__)
    FILE *clipboard = popen("pbcopy", "w");
    #else
    FILE *clipboard = popen("xclip -selection clipboard", "w");
    #endif
    if (clipboard == NULL) RETURNINTX(-4);
   // fprintf(clipboard, "%s", text);
    pclose(clipboard);
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(getclipboard) {
#if defined(__APPLE__)
    FILE *clipboard = popen("pbpaste", "r");
#else
    FILE *clipboard = popen("xclip -selection clipboard -o", "r");
#endif
    if (clipboard == NULL) {
        RETURNSTRX("$$$ERROR$$$");
    }
    
    char buffer[16000];
    memset(buffer, 0, sizeof(buffer));  // Initialize buffer
    
    if (fgets(buffer, sizeof(buffer) - 1, clipboard) != NULL) {
        pclose(clipboard);
        RETURNSTRX(buffer);
    }
    
    pclose(clipboard);
    RETURNSTRX("");
ENDPROC
}
#endif
PROCEDURE(setglobal) {
#if defined(_WIN32)
    if (_putenv_s(GETSTRING(ARG0), GETSTRING(ARG1)) == 0) {
#else
    if (setenv(GETSTRING(ARG0), GETSTRING(ARG1), 1) == 0) {
#endif
        RETURNINTX(0);
    } else {
        RETURNINTX(-8);
    }
ENDPROC
}

PROCEDURE(getglobal) {
   char *value = getenv(GETSTRING(ARG0));
   if (value) {
        RETURNSTRX(value);
    } else {
        RETURNSTRX("");
    }
ENDPROC
}

PROCEDURE(uptime) {
    // Get system uptime in milliseconds
    char result[64];
#if defined(_WIN32)
    ULONGLONG uptime = GetTickCount64();
    RETURNINTX(uptime/1000)
    // Convert uptime to seconds, minutes, hours, and days
 #elif defined(__linux__)
    #include <sys/sysinfo.h>
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        // Convert uptime to seconds, minutes, hours, and days
        long uptime_seconds = info.uptime;
        RETURNINTX(uptime_seconds)
     } else {
        RETURNINTX(result, "-1");
    }
#elif defined(__APPLE__)
    // Declare a struct to hold boot time information
    struct timeval boottime;
    size_t len = sizeof(boottime);

    // Get the boot time from sysctl
    if (sysctlbyname("kern.boottime", &boottime, &len, NULL, 0) == 0) {
        // Get the current time
        struct timeval now;
        gettimeofday(&now, NULL);

        // Calculate the uptime in seconds
        long uptime_seconds = now.tv_sec - boottime.tv_sec;
        RETURNINTX(uptime_seconds);
    }
#endif
    RETURNSTR(result);
ENDPROC
}
/* ------------------------------------------------------------------------------------------------
 * Wait pauses the execution (in milli seconds)
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(waitX) {
    int waittime = GETINT(ARG0);
    wait(waittime);
    RETURNINTX(0);
    ENDPROC
}
/* ------------------------------------------------------------------------------------------------
 * Beep creates a primitive beep
 * ------------------------------------------------------------------------------------------------
 */
PROCEDURE(beep) {
    int i;
//    for(i=0;i<5;i++) {
//        printf("\a");
//        wait(5);
//    }
    Beep(750, 300); // Frequency: 750 Hz, Duration: 300 ms
    RETURNINTX(0);
    ENDPROC
}
PROCEDURE(getuser) {
char username[256];
DWORD username_len = sizeof(username);


#ifdef _WIN32
    if (GetUserName(username, &username_len)) {
    RETURNSTRX(username);
    } else RETURNSTRX("unknown");
#else
   RETURNSTRX(getuid());
#endif
ENDPROC
}

PROCEDURE(getcomputer) {
    char hostname[256];
#ifdef _WIN32
    DWORD size = sizeof(hostname);
    if (GetComputerName(hostname, &size)) {
        RETURNSTRX(hostname);
    } else {
        RETURNSTRX("unknown");
    }
#else
    if (gethostname(hostname, sizeof(hostname)) == 0) {
       RETURNSTRX(hostname);
    } else {
       RETURNSTRX("unknown");
    }
#endif
ENDPROC
}
PROCEDURE(opsys) {
#ifdef _WIN32
 RETURNSTRX("Windows")
#elif __linux__
    RETURNSTRX("Linux")
#elif __APPLE__
   RETURNSTRX("macOS")
#elif __unix__
   RETURNSTRX("Unix")
#elif __FreeBSD__
   RETURNSTRX("FreeBSD")
#else
  RETURNSTRX("Unknown");
#endif
ENDPROC
}


/* -------------------------------------------------------------------------------------
 * Expose functions to CREXX
 * -------------------------------------------------------------------------------------
 */
LOADFUNCS
//      C Function, REXX namespace & name, Option, Return Type, Arguments
    ADDPROC(getEnv,      "system.getenv",      "b",    ".string", "env_name=.string");
    ADDPROC(getdir,      "system.getdir",      "b",    ".string", "");
    ADDPROC(setdir,      "system.setdir",      "b",    ".int",    "arg0=.string");
    ADDPROC(testdir,     "system.testdir",     "b",    ".int",    "arg0=.string");
    ADDPROC(createdir,   "system.createdir",   "b",    ".int",    "arg0=.string");
    ADDPROC(removedir,   "system.removedir",   "b",    ".int",    "arg0=.string");
    ADDPROC(deletefile,  "system.deletefile",  "b",    ".int",    "arg0=.string");
    ADDPROC(renamefileP, "system.renamefile",  "b",    ".int",    "arg0=.string,arg1=.string");
    ADDPROC(testfile,    "system.testfile",    "b",    ".int",    "arg0=.string");
    ADDPROC(listdir,     "system.listdir",     "b",    ".int",    "file=.string,expose entries=.string[]");
    ADDPROC(getclipboard,"system.getclipboard","b",    ".string","");
    ADDPROC(setclipboard,"system.setclipboard","b",    ".int","arg0=.string");
    ADDPROC(getglobal,   "system.getglobal"   ,"b",    ".string","key=.string");
    ADDPROC(setglobal,   "system.setglobal"   ,"b",    ".int",  "key=.string,value=.string");
    ADDPROC(uptime,      "system.uptime"      ,"b",    ".int",  "");
    ADDPROC(waitX,       "system.wait"        ,"b",    ".int",  "time=.int");
    ADDPROC(beep,        "system.beep"        ,"b",    ".int",  "");
    ADDPROC(getuser,     "system.userid"      ,"b",    ".string",  "");
    ADDPROC(getcomputer, "system.host"        ,"b",    ".string",  "");
    ADDPROC(opsys,       "system.opsys"       ,"b",    ".string",  "");
    ADDPROC(writeall,    "system.writeall",    "b",    ".int","expose array=.string[],file=.string,arg2=.int");
    ADDPROC(parse,       "system.parse",       "b",    ".int","string=.string,pattern=.string,expose variable=.string[],expose value=.string[]");

ENDLOADFUNCS
