//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#if defined(__APPLE__)
 #include <sys/stat.h>
#endif

#include <unistd.h>        // For POSIX systems (Linux/macOS)
#ifdef _WIN32
  #include <direct.h>     // For Windows
  #include <windows.h>
  #define getcwd _getcwd  // Map to Windows-specific version
#else
  #include <dirent.h>
#endif
#include "crexxpa.h"      // crexx/pa - Plugin Architecture header file
// distinguish between Windows and Linux and MAC
#if defined(_WIN32)
    #define REMOVE_DIR(path) _rmdir(path)
    #define MAKE_DIR(path)   _mkdir(path)
    #define TEST_DIR(path)   _access(path, 0)
    #define TEST_FILE(fname) _access(fname, 0)
#else
    #define REMOVE_DIR(path) rmdir(path)
    #define MAKE_DIR(path)   mkdir(path, 0755)
    #define TEST_DIR(path)    access(path, F_OK)
    #define TEST_FILE(fname) access(fname, F_OK)
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
    char *varName = GETSTRING(ARG0);    // Get the environment variable name
    char *varValue = getenv(varName);   // Get the environment variable value
    if (varValue == NULL) {             // If the environment variable is not found
        RETURNSIGNAL(SIGNAL_FAILURE, "Environment not found")
    } else {
        RETURNSTR(varValue);
    }
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
    searchReplace(GETSTRING(ARG0),'\\','/');
    if (TEST_FILE(GETSTRING(ARG0)) != 0)    RETURNINT(-4);             // does not exist
    else {
        printf("File '%s'\n",GETSTRING(ARG0));
        if (remove(GETSTRING(ARG0)) == 0) RETURNINT(0);
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
        RETURNINT(-8);
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
        RETURNINT(-8);
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
/* -------------------------------------------------------------------------------------
 * Expose functions to CREXX
 * -------------------------------------------------------------------------------------
 */
LOADFUNCS
//      C Function, REXX namespace & name, Option, Return Type, Arguments
    ADDPROC(getEnv,     "system.getenv",      "b",    ".string", "env_name=.string");
    ADDPROC(getdir,     "system.getdir",      "b",    ".string", "");
    ADDPROC(setdir,     "system.setdir",      "b",    ".int",    "arg0=.string");
    ADDPROC(testdir,    "system.testdir",     "b",    ".int",    "arg0=.string");
    ADDPROC(createdir,  "system.createdir",   "b",    ".int",    "arg0=.string");
    ADDPROC(removedir,  "system.removedir",   "b",    ".int",    "arg0=.string");
    ADDPROC(deletefile, "system.deletefile",  "b",    ".int",    "arg0=.string");
    ADDPROC(testfile,   "system.testfile",    "b",    ".int",    "arg0=.string");
    ADDPROC(listdir,    "system.listdir",     "b",    ".int",    "file=.string,expose entries=.string[]");

    ADDPROC(writeall,"system.writeall",   "b",  ".int", "expose array=.string[],file=.string,arg2=.int");
ENDLOADFUNCS
