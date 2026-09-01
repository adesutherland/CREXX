/* cREXX optional Level-G host-information and timing provider. */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#include <sys/time.h>
#endif

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#include "crexxpa.h"

#ifdef _WIN32

/**
 * @brief Convert a UTF-8 string to a Windows UTF-16 string.
 *
 * The returned buffer is allocated with malloc() and must be released
 * by the caller with free().
 *
 * @param value UTF-8 input string.
 * @return Allocated UTF-16 string, or NULL if conversion fails.
 */
static wchar_t *utf8_to_utf16(const char *value)
{
    int length;
    wchar_t *result;

    if (!value)
        return NULL;

    length = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value,
        -1,
        NULL,
        0);

    if (length <= 0)
        return NULL;

    result = (wchar_t *)malloc((size_t)length * sizeof(wchar_t));
    if (!result)
        return NULL;

    if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value,
            -1,
            result,
            length) <= 0) {
        free(result);
        return NULL;
            }

    return result;
}

#endif

#ifdef _WIN32
static char *powershell_quote(const char *value)
{
    size_t length = 3u;
    size_t i;
    size_t position = 0u;
    char *result;

    if (!value)
        value = "";
    for (i = 0u; value[i] != '\0'; ++i)
        length += (value[i] == '\'') ? 2u : 1u;
    result = (char *)malloc(length + 1u);
    if (!result)
        return NULL;
    result[position++] = '\'';
    for (i = 0u; value[i] != '\0'; ++i) {
        if (value[i] == '\'')
            result[position++] = '\'';
        result[position++] = value[i];
    }
    result[position++] = '\'';
    result[position] = '\0';
    return result;
}
#endif

#if !defined(_WIN32)

/* Quote one value for use as a single POSIX shell argument. */
static char *shell_quote(const char *value)
{
    size_t length = 2u;
    size_t i;
    size_t position = 0u;
    char *result;

    if (!value)
        value = "";

    for (i = 0u; value[i] != '\0'; ++i)
        length += (value[i] == '\'') ? 4u : 1u;

    result = (char *)malloc(length + 1u);
    if (!result)
        return NULL;

    result[position++] = '\'';
    for (i = 0u; value[i] != '\0'; ++i) {
        if (value[i] == '\'') {
            result[position++] = '\'';
            result[position++] = '\\';
            result[position++] = '\'';
            result[position++] = '\'';
        } else {
            result[position++] = value[i];
        }
    }
    result[position++] = '\'';
    result[position] = '\0';
    return result;
}

#ifdef __APPLE__

/* Quote text as an AppleScript string, then pass it through shell_quote(). */
static char *applescript_quote(const char *value)
{
    size_t length = 3u;
    size_t i;
    size_t position = 0u;
    char *result;

    if (!value)
        value = "";
    for (i = 0u; value[i] != '\0'; ++i)
        length += (value[i] == '\\' || value[i] == '"') ? 2u : 1u;

    result = (char *)malloc(length + 1u);
    if (!result)
        return NULL;
    result[position++] = '"';
    for (i = 0u; value[i] != '\0'; ++i) {
        if (value[i] == '\\' || value[i] == '"')
            result[position++] = '\\';
        result[position++] = value[i];
    }
    result[position++] = '"';
    result[position] = '\0';
    return result;
}

#endif

#endif

RXPA_PLUGIN_PROCESS_REENTRANT

PROCEDURE(uptime)
{
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.UPTIME expects no arguments")
#ifdef _WIN32
    RETURNINT((rxinteger)(GetTickCount64() / 1000u));
#elif defined(__linux__)
    {
        struct sysinfo info;
        if (sysinfo(&info) != 0)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.UPTIME is unavailable")
        RETURNINT((rxinteger)info.uptime);
    }
#elif defined(__APPLE__)
    {
        struct timeval boot;
        struct timeval now;
        size_t length = sizeof(boot);
        if (sysctlbyname("kern.boottime", &boot, &length, NULL, 0) != 0 ||
            gettimeofday(&now, NULL) != 0)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.UPTIME is unavailable")
        RETURNINT((rxinteger)(now.tv_sec - boot.tv_sec));
    }
#else
    RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.UPTIME is unavailable")
#endif
    RESETSIGNAL
}

PROCEDURE(user)
{
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.USER expects no arguments")
#ifdef _WIN32
    {
        char name[256];
        DWORD length = (DWORD)sizeof(name);
        if (!GetUserNameA(name, &length))
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.USER is unavailable")
        RETURNSTR(name);
    }
#else
    {
        struct passwd pwd;
        struct passwd *result = NULL;
        char buffer[16384];
        const char *name = NULL;
        if (getpwuid_r(getuid(), &pwd, buffer, sizeof(buffer), &result) == 0 &&
            result && result->pw_name && result->pw_name[0]) name = result->pw_name;
        if (!name || !name[0]) name = getenv("USER");
        if (!name || !name[0]) name = getenv("LOGNAME");
        if (!name || !name[0])
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.USER is unavailable")
        RETURNSTR(name);
    }
#endif
    RESETSIGNAL
}

PROCEDURE(host)
{
    char name[256];
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.HOST expects no arguments")
#ifdef _WIN32
    {
        DWORD length = (DWORD)sizeof(name);
        if (!GetComputerNameA(name, &length))
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.HOST is unavailable")
    }
#else
    if (gethostname(name, sizeof(name)) != 0)
        RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.HOST is unavailable")
    name[sizeof(name) - 1u] = '\0';
#endif
    RETURNSTR(name);
    RESETSIGNAL
}

PROCEDURE(osname)
{
    if (NUM_ARGS != 0)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.OSNAME expects no arguments")
#ifdef _WIN32
    RETURNSTR("Windows");
#elif defined(__linux__)
    RETURNSTR("Linux");
#elif defined(__APPLE__)
    RETURNSTR("macOS");
#elif defined(__FreeBSD__)
    RETURNSTR("FreeBSD");
#elif defined(__unix__)
    RETURNSTR("Unix");
#else
    RETURNSTR("Unknown");
#endif
    RESETSIGNAL
}

PROCEDURE(sleep_ms)
{
    rxinteger milliseconds;
    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.SLEEP expects one millisecond value")
    milliseconds = GETINT(ARG0);
    if (milliseconds < 0 || (uint64_t)milliseconds > UINT32_MAX)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS, "RXPLATFORM.SLEEP requires 0..4294967295 milliseconds")
#ifdef _WIN32
    Sleep((DWORD)milliseconds);
#else
    {
        struct timespec requested;
        struct timespec remaining;
        requested.tv_sec = (time_t)(milliseconds / 1000);
        requested.tv_nsec = (long)((milliseconds % 1000) * 1000000);
        while (nanosleep(&requested, &remaining) != 0) requested = remaining;
    }
#endif
    RETURNINT(0);
    RESETSIGNAL
}

/**
 * @brief Display an informational message to the user.
 *
 * Rexx:
 *   call rxplatform.message("Processing complete")
 *
 * @return 0 on success.
 */
PROCEDURE(message)
{
    const char *text;
    if (NUM_ARGS != 1)
        RETURNSIGNAL(
            SIGNAL_INVALID_ARGUMENTS,
            "RXPLATFORM.MESSAGE expects one message argument")

    text = GETSTRING(ARG0);
#ifdef _WIN32
    {
        wchar_t *wide_text;

        wide_text = utf8_to_utf16(text);
        if (!wide_text)
            RETURNSIGNAL(
                SIGNAL_FAILURE,
                "RXPLATFORM.MESSAGE cannot convert message to Unicode")
        MessageBoxW(
            NULL,
            wide_text,
            L"cREXX",
            MB_OK | MB_ICONINFORMATION);
        free(wide_text);
    }
#elif defined(__APPLE__)
    {
        char *script_text = applescript_quote(text);
        char *quoted_command;
        char *full_command;
        size_t command_length;
        int status;

        if (!script_text)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.MESSAGE cannot allocate dialog command")
        command_length = strlen("display dialog ") + strlen(script_text) +
                         strlen(" buttons {\"OK\"}");
        {
            char *script_command = (char *)malloc(command_length + 1u);
            if (!script_command) {
                free(script_text);
                RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.MESSAGE cannot allocate dialog command")
            }
            snprintf(script_command, command_length + 1u,
                     "display dialog %s buttons {\"OK\"}", script_text);
            quoted_command = shell_quote(script_command);
            free(script_command);
        }
        if (!quoted_command) {
            free(script_text);
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.MESSAGE cannot allocate dialog command")
        }
        command_length = strlen("osascript -e ") + strlen(quoted_command) + 1u;
        full_command = (char *)malloc(command_length);
        if (!full_command) {
            free(quoted_command);
            free(script_text);
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.MESSAGE cannot allocate dialog command")
        }
        snprintf(full_command, command_length,
                 "osascript -e %s", quoted_command);
        status = system(full_command);
        free(full_command);
        free(quoted_command);
        free(script_text);
        if (status == -1 || status != 0)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.MESSAGE dialog failed")
    }
#elif defined(__linux__)
    {
        char *quoted_text = shell_quote(text);
        size_t command_length;
        char *command;
        int status;

        if (!quoted_text)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.MESSAGE cannot allocate dialog command")
        command_length = strlen("zenity --info --title='cREXX' --text=") + strlen(quoted_text) + 1u;
        command = (char *)malloc(command_length);
        if (!command) {
            free(quoted_text);
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.MESSAGE cannot allocate dialog command")
        }
        snprintf(command, command_length, "zenity --info --title='cREXX' --text=%s", quoted_text);
        if (system("command -v zenity >/dev/null 2>&1") == 0) {
            status = system(command);
        } else if (system("command -v kdialog >/dev/null 2>&1") == 0) {
            snprintf(command, command_length, "kdialog --title 'cREXX' --msgbox %s", quoted_text);
            status = system(command);
        } else {
            status = -1;
        }
        free(command);
        free(quoted_text);
        if (status == -1 || status != 0)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.MESSAGE requires zenity or kdialog")
    }
#else
    RETURNSIGNAL(
        SIGNAL_FAILURE,
        "RXPLATFORM.MESSAGE is not yet implemented on this platform")
#endif

    RETURNINT(0);
    ENDPROC
}


/**
 * @brief Request one line of text from the user.
 *
 * Cancellation is represented by the empty string.
 */
PROCEDURE(input)
{
    const char *prompt;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(SIGNAL_INVALID_ARGUMENTS,
                     "RXPLATFORM.INPUT expects one prompt argument")
    prompt = GETSTRING(ARG0);
#ifdef _WIN32
    {
        char *quoted_prompt = powershell_quote(prompt);
        char command[8192];
        FILE *pipe;
        char response[8192];
        if (!quoted_prompt)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT cannot allocate dialog command")
        snprintf(command, sizeof(command),
                 "powershell.exe -NoProfile -NonInteractive -Command "
                 "\"Add-Type -AssemblyName Microsoft.VisualBasic; "
                 "[Microsoft.VisualBasic.Interaction]::InputBox(%s,'cREXX','')\"",
                 quoted_prompt);
        free(quoted_prompt);
        pipe = _popen(command, "rt");
        if (!pipe)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT dialog failed")
        response[0] = '\0';
        (void)fgets(response, sizeof(response), pipe);
        (void)_pclose(pipe);
        response[strcspn(response, "\r\n")] = '\0';
        RETURNSTRX(response);
    }
#elif defined(__APPLE__)
    {
        char *script_text = applescript_quote(prompt);
        char *script;
        char *quoted_script;
        size_t length;
        char command[16384];
        FILE *pipe;
        char response[8192];
        char *value;
        if (!script_text)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT cannot allocate dialog command")
        length = strlen("display dialog ") + strlen(script_text) +
                 strlen(" default answer \"\" buttons {\"Cancel\",\"OK\"} default button \"OK\" cancel button \"Cancel\"") + 1u;
        script = (char *)malloc(length);
        if (!script) {
            free(script_text);
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT cannot allocate dialog command")
        }
        snprintf(script, length,
                 "display dialog %s default answer \"\" buttons {\"Cancel\",\"OK\"} default button \"OK\" cancel button \"Cancel\"",
                 script_text);
        free(script_text);
        quoted_script = shell_quote(script);
        free(script);
        if (!quoted_script)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT cannot allocate dialog command")
        snprintf(command, sizeof(command), "osascript -e %s", quoted_script);
        free(quoted_script);
        pipe = popen(command, "r");
        if (!pipe)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT dialog failed")
        response[0] = '\0';
        (void)fgets(response, sizeof(response), pipe);
        (void)pclose(pipe);
        value = strstr(response, "text returned:");
        if (!value)
            RETURNSTRX("");
        value += strlen("text returned:");
        value[strcspn(value, "\r\n")] = '\0';
        RETURNSTRX(value);
    }
#elif defined(__linux__)
    {
        char *quoted_prompt = shell_quote(prompt);
        char command[8192];
        FILE *pipe;
        char response[8192];
        if (!quoted_prompt)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT cannot allocate dialog command")
        if (system("command -v zenity >/dev/null 2>&1") == 0)
            snprintf(command, sizeof(command), "zenity --entry --title='cREXX' --text=%s", quoted_prompt);
        else if (system("command -v kdialog >/dev/null 2>&1") == 0)
            snprintf(command, sizeof(command), "kdialog --title 'cREXX' --inputbox %s", quoted_prompt);
        else {
            free(quoted_prompt);
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT requires zenity or kdialog")
        }
        free(quoted_prompt);
        pipe = popen(command, "r");
        if (!pipe)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT dialog failed")
        response[0] = '\0';
        (void)fgets(response, sizeof(response), pipe);
        (void)pclose(pipe);
        response[strcspn(response, "\r\n")] = '\0';
        RETURNSTRX(response);
    }
#else
    RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.INPUT is not implemented on this platform")
#endif
    ENDPROC
    RESETSIGNAL
}

/**
 * @brief Ask the user to confirm an operation.
 *
 * Rexx:
 *   answer = rxplatform.confirm("Delete existing output?")
 *
 * @return 1 if Yes was selected, otherwise 0.
 */
PROCEDURE(confirm)
{
    const char *text;

    if (NUM_ARGS != 1)
        RETURNSIGNAL(
            SIGNAL_INVALID_ARGUMENTS,
            "RXPLATFORM.CONFIRM expects one message argument")

    text = GETSTRING(ARG0);

#ifdef _WIN32
    {
        wchar_t *wide_text;
        int result;

        wide_text = utf8_to_utf16(text);
        if (!wide_text)
            RETURNSIGNAL(
                SIGNAL_FAILURE,
                "RXPLATFORM.CONFIRM cannot convert message to Unicode")

        result = MessageBoxW(
            NULL,
            wide_text,
            L"cREXX",
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);

        free(wide_text);
        printf("Confirm %s %d\n",text,result);
        if (result == IDYES)
            RETURNINTX(1);

        if (result == IDNO)
            RETURNINTX(0);

        RETURNSIGNAL(
            SIGNAL_FAILURE,
            "RXPLATFORM.CONFIRM failed")
    }
#elif defined(__APPLE__)
    {
        char *script_text = applescript_quote(text);
        char *quoted_text;
        char *script;
        size_t command_length;
        char *command;
        FILE *pipe;
        char response[256];

        if (!script_text)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM cannot allocate dialog command")
        command_length = strlen("display dialog ") + strlen(script_text) +
                         strlen(" buttons {\"No\",\"Yes\"} default button \"No\"") + 1u;
        script = (char *)malloc(command_length);
        if (!script) {
            free(script_text);
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM cannot allocate dialog command")
        }
        snprintf(script, command_length,
                 "display dialog %s buttons {\"No\",\"Yes\"} default button \"No\"",
                 script_text);
        free(script_text);
        quoted_text = shell_quote(script);
        free(script);
        if (!quoted_text)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM cannot allocate dialog command")
        command_length = strlen("osascript -e ") + strlen(quoted_text) + 1u;
        command = (char *)malloc(command_length);
        if (!command) {
            free(quoted_text);
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM cannot allocate dialog command")
        }
        snprintf(command, command_length,
                 "osascript -e %s",
                 quoted_text);
        pipe = popen(command, "r");
        free(command);
        free(quoted_text);
        if (!pipe)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM dialog failed")
        response[0] = '\0';
        (void)fgets(response, sizeof(response), pipe);
        if (pclose(pipe) != 0)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM dialog failed")
        if (strstr(response, "button returned:Yes") != NULL)
            RETURNINTX(1);
        RETURNINTX(0);
    }
#elif defined(__linux__)
    {
        char *quoted_text = shell_quote(text);
        size_t command_length;
        char *command;
        int status;

        if (!quoted_text)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM cannot allocate dialog command")
        command_length = strlen("zenity --question --title='cREXX' --text=") + strlen(quoted_text) + 1u;
        command = (char *)malloc(command_length);
        if (!command) {
            free(quoted_text);
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM cannot allocate dialog command")
        }
        snprintf(command, command_length, "zenity --question --title='cREXX' --text=%s", quoted_text);
        if (system("command -v zenity >/dev/null 2>&1") == 0) {
            status = system(command);
        } else if (system("command -v kdialog >/dev/null 2>&1") == 0) {
            snprintf(command, command_length, "kdialog --title 'cREXX' --yesno %s", quoted_text);
            status = system(command);
        } else {
            status = -1;
        }
        free(command);
        free(quoted_text);
        if (status == -1)
            RETURNSIGNAL(SIGNAL_FAILURE, "RXPLATFORM.CONFIRM requires zenity or kdialog")
        RETURNINTX(status == 0 ? 1 : 0);
    }
#else
    RETURNSIGNAL(
        SIGNAL_FAILURE,
        "RXPLATFORM.CONFIRM is not yet implemented on this platform")
#endif
    ENDPROC
    RESETSIGNAL

}

LOADFUNCS
    ADDPROC(uptime, "rxplatform.uptime", "b", ".int", "");
    ADDPROC(user, "rxplatform.user", "b", ".string", "");
    ADDPROC(host, "rxplatform.host", "b", ".string", "");
    ADDPROC(osname, "rxplatform.osname", "b", ".string", "");
    ADDPROC(sleep_ms, "rxplatform.sleep", "b", ".int", "milliseconds = .int");
    ADDPROC(message, "rxplatform.message", "b", ".int", "message = .string");
    ADDPROC(confirm, "rxplatform.confirm", "b", ".int", "message = .string");
    ADDPROC(input, "rxplatform.input", "b", ".string", "prompt = .string");
ENDLOADFUNCS
