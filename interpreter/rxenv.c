/* cREXX License (MIT)
 * Copyright (c) 2020-2026 Adrian Sutherland, Peter Jacob, Rene Jansen
 * Environment access is independent of process and thread creation.
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef CREXX_NATIVE_RAW_IO
#include "platform_native.h"
#endif
#ifdef _WIN32
#include <windows.h>
#endif

/* Get Environment Value
 * Sets value (null terminated) (and a handle) from env variable name length name_length (not null terminated)
 * Value can be set to point to a zero length string (if the variable is not set)
 *
 * Returns 1 if value should be free()d, 0 for borrowed/absent values, or
 * -1/errno when the native environment service fails.
 */
int getEnvVal(char **value, char *name, size_t name_length) {

    char* nulled_name;
    if (!value || (!name && name_length)) { errno = EINVAL; return -1; }
    *value = NULL;
    if (!name_length) {
        *value = "";
        return 0;
    }
#ifdef CREXX_NATIVE_RAW_IO
    if (memchr(name, 0, name_length)) { errno = EINVAL; return -1; }
#endif
    if (name_length == (size_t)-1) { errno = EOVERFLOW; return -1; }
    nulled_name = malloc(name_length + 1);
    if (!nulled_name) { errno = ENOMEM; return -1; }
    memcpy(nulled_name, name, name_length);
    nulled_name[name_length] = 0;

#ifdef CREXX_NATIVE_RAW_IO
    {
        int result = crexx_native_environment(nulled_name, value);
        int error = errno;
        free(nulled_name);
        if (result < 0) { errno = error; return -1; }
        if (!result) { *value = ""; return 0; }
        return 1;
    }
#elif defined(_WIN32)

    wchar_t *wname;
    int wname_length = MultiByteToWideChar(CP_UTF8, 0, nulled_name, -1, NULL, 0);
    wname = (wchar_t *)malloc(wname_length * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, nulled_name, -1, wname, wname_length);

    DWORD len = GetEnvironmentVariableW(wname, NULL, 0);
    if (len > 0) {
        wchar_t *wvalue = (wchar_t *)malloc(len * sizeof(wchar_t));
        GetEnvironmentVariableW(wname, wvalue, len);

        int utf8_length = WideCharToMultiByte(CP_UTF8, 0, wvalue, len, NULL, 0, NULL, NULL);
        *value = malloc(utf8_length + 1);
        WideCharToMultiByte(CP_UTF8, 0, wvalue, len, *value, utf8_length, NULL, NULL);
        (*value)[utf8_length] = '\0';

        free(wvalue);
    }
    else {
        *value = "";
    }
    free(wname);
    free(nulled_name);
    return len > 0 ? 1 : 0;

#else

    *value = getenv(nulled_name);
    free(nulled_name);
    if (!(*value)) {
        *value = "";
    }
    return 0;

#endif
}
