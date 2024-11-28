//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <math.h>
#include "windows.h"

void searchReplace(char *str, char search, char replace) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {  // Loop until the end of the string
        if (str[i] == search) {
            str[i] = replace;  // Replace the character
        }
    }
}
PROCEDURE(readall) {
    int lines=1,llen,amax=0,amaxl,maxlines;
    searchReplace(GETSTRING(ARG1),'\\','/');
    maxlines = GETINT(ARG2);
    FILE *file = fopen(GETSTRING(ARG1), "r");
    if (file == NULL) {
        RETURNINT(-8);
        PROCRETURN
    }
    char line[32000];      // 32k should be enough
    amaxl=250;             // define new junk to allow additional lines in exceeded array
    while (fgets(line, sizeof(line), file) != NULL) {
       llen=strlen(line);
       if (line[llen-1]=='\n') line[llen-1]='\0';
       if (amax==0) {
           amax=amaxl;
           SETARRAYHI(ARG0,GETARRAYHI(ARG0)+amaxl);
        }
        SETSARRAY(ARG0,lines-1,line);   // push into array

        if (maxlines>0 && lines>=maxlines) break;
        lines++;
        amax--;
     }
    SETARRAYHI(ARG0,lines);  // adjust arrayhi
    RETURNINT(lines);
    PROCRETURN
    ENDPROC
}
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
    PROCRETURN
    ENDPROC
}
PROCEDURE(readdir) {
    int indx1=0,indx2=0;
    char vname[512];
    WIN32_FIND_DATA findFileData;

    memset(vname, 0, sizeof(vname));
    searchReplace(GETSTRING(ARG2),'\\','/');
    sprintf(vname, "%s%s",GETSTRING(ARG2),"/*");

    HANDLE hFind = FindFirstFile(vname, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        RETURNINT(-8);
        PROCRETURN;
    }
    do {
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (findFileData.cFileName[0]=='.') continue;  // drop . and .. entries
            SETARRAYHI(ARG0,indx1+1);
            SETSARRAY(ARG0,indx1,findFileData.cFileName);
            indx1++;
        } else {
            SETARRAYHI(ARG1,indx2+1);
            SETSARRAY(ARG1,indx2,findFileData.cFileName);
            indx2++;
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    RETURNINT(0);
    PROCRETURN;
ENDPROC
}
// File IO function definitions
LOADFUNCS
    ADDPROC(readall,"fileio.readall",   "b",  ".int", "expose array=.string[],file=.string,arg2=.int");
    ADDPROC(writeall,"fileio.writeall",   "b",  ".int", "expose array=.string[],file=.string,arg2=.int");
    ADDPROC(readdir,"fileio.readdir",   "b",  ".int", "expose entries=.string[],expose dirs=.string[],file=.string");
ENDLOADFUNCS
