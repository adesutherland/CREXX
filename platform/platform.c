//
// Std C - Utility Functions
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef _WIN32
# include <dirent.h>
# include <glob.h>
#endif


#include "platform.h"

static int hasWildcards(const char *fileName);

/*
 * Read a file into a returned buffer
 *
 * This function malloc()s the buffer to the right size therefore it needs
 * to be free()d by the caller
 */
char* file2buf(FILE *file) {
    size_t bytes;
    char *buff;

    /* Get file size */
    fseek(file, 0, SEEK_END);
    bytes = ftell(file);
    rewind(file);

    /* Allocate buffer and read */
    buff = (char*) malloc((bytes + 1) * sizeof(char) );
    bytes = fread(buff, 1, bytes, file);
    if (!bytes) {
        fprintf(stderr, "Error reading input file\n");
        exit(-1);
    }
    buff[bytes] = 0;

    return buff;
}

/*
 * Function opens and returns a file handle
 * dir can be null - the default is platform specific (e.g. ./ )
 * mode - is the fopen() file mode
 */
FILE *openfile(char *name, char *type, char *dir, char *mode) {
    size_t len;
    char *file_name;
    FILE *stream;
    if (!dir) dir = ".";

    len = strlen(name) + strlen(type) + strlen(dir) + 3;

    file_name = malloc(len);
    snprintf(file_name, len, "%s/%s.%s", dir, name, type);

    stream = fopen(file_name, mode);

    free(file_name);

    return stream;
}

/*
 * Function returns the extension part of a file name, if present.
 * Otherwise, it returns NULL.
 */
const char *fnext(const char *file_name) {
    const char *dot = strrchr(file_name, '.');

    if(!dot || dot == file_name) {
        return NULL;
    } else {
        return dot + 1;
    }
}

/*
 * Function to create the internal VFILE structure from a given input file name.
 * If given file name did not have a path, the optional default path will be used.
 * If the default path is NULL the local directory path './' will be used.
 * If given file name did not have an extension, the default extension will be used.
 *
 * If the given file name contains wildcards, a list of VFILEs will be created.
 */
VFILE* vfnew(const char *inputName, VFILE *vfile, const char *defaultPath, const char *defaultExtension) {

    size_t pathLength, basenameLength, extensionLength, fullLength;

    const char *pathEndPtr, *baseName ;
    char *cpyPtr;

    if (inputName == NULL) {
        fprintf(stderr, "Internal error while creating  VFILE structure. InputName is NULL!");
        exit(42);
    }

#ifdef _WIN32
    if (hasWildcards(inputName)) {
        fprintf(stderr, "Wildcards in PATH are not supported on this platform.");
        exit(-1);
    }
#endif

    // find last file separator
    pathEndPtr = strrchr(inputName, FILE_SEPARATOR);

    // path present
    if (pathEndPtr > 0) {

        // save the real pointer to basename
        baseName = pathEndPtr + 1;

        pathLength = baseName - inputName;

        // copy library path
        vfile->path = calloc(1, pathLength + 1 /* EOS */);
        memcpy(vfile->path, inputName, pathLength);

    } else {
        // default path given
        if (defaultPath) {
            pathLength = strlen(defaultPath);

            // check for trailing file separator and append if necessary
            if (defaultPath[pathLength - 1] != FILE_SEPARATOR) {
                pathLength++;
            }

            vfile->path = calloc(1, pathLength + 1 /* EOS */);

            memcpy(vfile->path, defaultPath, pathLength);

            // make sure path ends with a file separator
            vfile->path[pathLength - 1] = FILE_SEPARATOR;

        } else {
            // to hold "./"
            pathLength = 2;

            vfile->path = calloc(1, pathLength + 1 /* EOS */);
            snprintf(vfile->path, pathLength + 1, "%c%c", '.', FILE_SEPARATOR);
        }

        baseName = inputName;
    }

    basenameLength = strlen(baseName);
    if ((baseName)) {
        if (fnext(baseName)) {
            basenameLength = basenameLength - (strlen(fnext(baseName)) + 1 /* the dot */) ;
        }
    }

    // copy library name
    vfile->basename = calloc(1, basenameLength + 1 /* EOS */);
    memcpy(vfile->basename, baseName, basenameLength);

    if (fnext(baseName)) {
        extensionLength = strlen(fnext(baseName)) + 1 /* the dot */;
    } else {
        extensionLength = strlen(defaultExtension);
    }

    // copy library extension
    vfile->extension = calloc(1, extensionLength + 1 /* EOS */);
    if (fnext(baseName)) {
        memcpy(vfile->extension, fnext(baseName), extensionLength);
    } else {
        memcpy(vfile->extension, defaultExtension, extensionLength);
    }

    // creating full name
    fullLength = pathLength + basenameLength + extensionLength + 1 /* the DOT */;

    vfile->fullname = calloc( 1, fullLength + 1 /* EOS */);

    // needed for arithmetics
    cpyPtr = vfile->fullname;

    memcpy(cpyPtr, vfile->path, pathLength);
    cpyPtr += pathLength;

    memcpy(cpyPtr, vfile->basename, basenameLength);
    cpyPtr += basenameLength;

    memcpy(cpyPtr, ".", 1);
    cpyPtr++;

    memcpy(cpyPtr, vfile->extension, extensionLength);

    if (access(vfile->fullname, F_OK) == 0) {
        vfile->exists = 1;
    } else {
        vfile->exists = 0;
    }

#ifndef _WIN32
    if (hasWildcards(vfile->fullname)) {

        int ii;

        VFILE *current;
        current = vfile;

        // mark as wildcarded
        current->wildcarded = 1;

        glob_t globResult;

        glob(vfile->fullname, GLOB_TILDE, NULL, &globResult);

        for (ii = 0; ii < globResult.gl_pathc; ii++) {
            current->next = calloc(1, sizeof(VFILE));
            current->next = vfnew(globResult.gl_pathv[ii], current->next, defaultPath, defaultExtension);
            current = current->next;
        }

    }
#endif

    return vfile;
}

/*
 * Function to open a VFILE in given mode.
 * mode - is the fopen() file mode
 */
VFILE* vfopen(VFILE *vfile, char *mode) {
    if (vfile) {
        vfile->fp = fopen(vfile->fullname, mode);

        if (vfile->fp) {
            vfile->exists = 1;
            vfile->opened = 1;
        }
    }

    return vfile;
}

/*
 * Function to close a single open VFILE.
 */
void vfclose(VFILE *vfile) {
    if (vfile) {
        if (vfile->fp) {
            if (ftell(vfile->fp) >= 0) {
                fclose(vfile->fp);
                vfile->opened = 0;
            }
        }
    }
}

/*
 * Function to close all eventually open file pointer and
 * free all allocated memory for given VFILE structure.
 */
void vffree(VFILE *vfile) {

    while(vfile) {
        vfclose(vfile);

        if (vfile->path) {
            free(vfile->path);
        }

        if (vfile->basename) {
            free(vfile->basename);
        }

        if (vfile->extension) {
            free(vfile->extension);
        }

        if (vfile->fullname) {
            free(vfile->fullname);
        }

        vfile = vfile->next;
    }

}

static int hasWildcards(const char *fileName) {
    int hasWildcard;

    char tilde        = '~';
    char questionMark = '?';
    char asterisk     = '*';
    char oBracket     = '[';
    char cBracket     = ']';

    hasWildcard = strchr(fileName, tilde) != NULL;

    if (!hasWildcard)
        hasWildcard = strchr(fileName, questionMark) != NULL;
    if (!hasWildcard)
        hasWildcard = strchr(fileName, asterisk) != NULL;
    if (!hasWildcard)
        hasWildcard = strchr(fileName, oBracket) != NULL;
    if (!hasWildcard)
        hasWildcard = strchr(fileName, cBracket) != NULL;

    return hasWildcard;
}
