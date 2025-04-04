#include <stdio.h>
#include "crexxpa.h"      // crexx/pa - Plugin Architecture header file
#ifdef _WIN32
#include <direct.h>     // For Windows
#include <windows.h>
#define wait(ms) Sleep(ms);
#elif defined(__APPLE__)
#else
// #include <arpa/inet.h>    // Linux
   #define wait(ms) usleep(ms*1000)
#endif
#ifdef _WIN32  // Windows-specific
#include <conio.h>
#define GETCH() _getch()

#else  // Linux/macOS
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

char getch() {
    struct termios oldt, newt;
    char ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);  // Get current terminal attributes
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);  // Disable line buffering & echo
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Apply new settings

    // Set stdin to non-blocking mode
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    // Read one character (will return immediately if no character is available)
    ch = getchar();

    // Restore blocking mode
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    
    // Restore old terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    return ch;
}

#define GETCH() getch()  // Define macro for Linux/macOS
#endif

/* GETCHAR - Get a character without echo, with IDE fallback */
PROCEDURE(GETCHAR) {
    char ch = 0;
    int timeout = -1;  // Default: wait forever
    
    // Check if timeout parameter was provided
    timeout = GETINT(ARG0);  // Timeout in milliseconds

#ifdef _WIN32
    // Try to get character using _getch
    if (timeout <= 0) {
        // No timeout - standard behavior
        ch = _getch();
    } else {
        // With timeout - poll for input
        DWORD startTime = GetTickCount();
        while (GetTickCount() - startTime < (DWORD)timeout) {
            if (_kbhit()) {
                ch = _getch();
                break;
            }
            Sleep(10);  // Small delay to prevent CPU hogging
        }
    }

#else
    // Unix implementation
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    if (timeout <= 0) {
        // No timeout - read directly
        ch = getchar();
    } else {
        // With timeout - use select for timeout
        fd_set readfds;
        struct timeval tv;
        
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        tv.tv_sec = timeout / 1000;
        tv.tv_usec = (timeout % 1000) * 1000;
        
        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
            ch = getchar();
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

#endif

    RETURNINTX((int)ch);
ENDPROC
}

/* READSTRING - Read a string from the console */
PROCEDURE(READSTRING) {
    char buffer[1024] = {0};
    int maxLen = 1023;  // Leave room for null terminator
    
    // Check if max length parameter was provided

        int requestedLen = GETINT(ARG0);
        if (requestedLen > 0 && requestedLen < maxLen) {
            maxLen = requestedLen;
        }

    // Prompt if provided
       char * prompt=GETSTRING(ARG1);
       if (strlen(prompt)>0) {
           printf("%s", GETSTRING(ARG1));
           fflush(stdout);
       }

    
    // Read input
    if (fgets(buffer, maxLen, stdin) != NULL) {
        // Remove trailing newline if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
    RETURNSTRX(buffer);
 ENDPROC
}

/* KBHIT - Check if a key has been pressed */
PROCEDURE(KBHIT) {
    int result = 0;
    
#ifdef _WIN32
    result = _kbhit();
#else
    struct termios oldt, newt;
    int oldf;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    
    int ch = getchar();
    
    if (ch != EOF) {
        ungetc(ch, stdin);
        result = 1;
    }
    
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    RETURNINTX(result);
ENDPROC
}

/* GETKEY - Wait for a key press and return its ASCII value */
PROCEDURE(GETKEY) {
    printf("Press any key...");
    fflush(stdout);
    
    int key = GETCH();
    
    printf("\nKey pressed: %c (ASCII: %d)\n", key, key);
    
    RETURNINTX(key);
ENDPROC
}

/* SETCURSOR - Set cursor position */
PROCEDURE(SETCURSOR) {
    int row = 0, col = 0;
    row=GETINT(ARG0);
    col=GETINT(ARG1);

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {col, row};
    SetConsoleCursorPosition(hConsole, pos);
#else
    printf("\033[%d;%dH", row+1, col+1);
    fflush(stdout);
#endif
    RETURNINTX(0);
ENDPROC
}

/* CLRSCR - Clear the screen */
PROCEDURE(CLRSCR) {
#ifdef _WIN32
    // Try the direct Windows API approach first
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    COORD homeCoords = {0, 0};

    if (hConsole != INVALID_HANDLE_VALUE && 
        GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        
        DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
        
        // Fill with spaces and reset attributes
        FillConsoleOutputCharacter(hConsole, (TCHAR)' ', cellCount, homeCoords, &count);
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, cellCount, homeCoords, &count);
        
        // Move cursor to home position
        SetConsoleCursorPosition(hConsole, homeCoords);
    } else {
        // Fallback to system command if API approach fails
        system("cls");
    }
#else
    // For Unix systems, use both ANSI escape sequences and system command for reliability
    printf("\033[2J\033[H");  // ANSI: Clear screen and move cursor to home
    fflush(stdout);
    
    // Fallback to system command if needed
    system("clear");
#endif

    RETURNINTX(0);
ENDPROC
}

/* SETCOLOR - Set text and/or background color */
PROCEDURE(SETCOLOR) {
    int fgColor = -1;  // -1 means don't change
    int bgColor = -1;  // -1 means don't change
    
    // Get parameters
    fgColor = GETINT(ARG0);
    bgColor = GETINT(ARG1);
    
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
    GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
    int currentAttrs = consoleInfo.wAttributes;
    
    // Calculate new attributes
    int newAttrs = currentAttrs;
    
    // Update foreground color if specified
    if (fgColor >= 0 && fgColor <= 15) {
        // Clear foreground bits and set new ones
        newAttrs = (newAttrs & 0xF0) | fgColor;
    }
    
    // Update background color if specified
    if (bgColor >= 0 && bgColor <= 15) {
        // Clear background bits and set new ones
        newAttrs = (newAttrs & 0x0F) | (bgColor << 4);
    }
    
    // Apply the new attributes
    SetConsoleTextAttribute(hConsole, newAttrs);
#else
    // For Unix-like systems, use ANSI escape sequences
    
    // Build the escape sequence
    char escapeSeq[20] = "\033[";
    char temp[10];
    int hasColor = 0;
    
    // Reset colors if both are specified
    if (fgColor >= 0 && bgColor >= 0) {
        strcat(escapeSeq, "0;");  // Reset first
    }
    
    // Add foreground color if specified
    if (fgColor >= 0 && fgColor <= 15) {
        // Convert color code to ANSI
        if (fgColor >= 8) {
            // Bright colors
            sprintf(temp, "1;%d", 30 + (fgColor & 7));
        } else {
            // Normal colors
            sprintf(temp, "%d", 30 + fgColor);
        }
        strcat(escapeSeq, temp);
        hasColor = 1;
    }
    
    // Add background color if specified
    if (bgColor >= 0 && bgColor <= 15) {
        if (hasColor) {
            strcat(escapeSeq, ";");
        }
        
        // Convert color code to ANSI
        if (bgColor >= 8) {
            // Bright background (not widely supported)
            sprintf(temp, "1;%d", 40 + (bgColor & 7));
        } else {
            // Normal background
            sprintf(temp, "%d", 40 + bgColor);
        }
        strcat(escapeSeq, temp);
        hasColor = 1;
    }
    
    // Finish and output the escape sequence if any colors were specified
    if (hasColor) {
        strcat(escapeSeq, "m");
        printf("%s", escapeSeq);
        fflush(stdout);
    }
#endif
    
    RETURNINTX(0);
ENDPROC
}

/* RESETCOLORS - Reset colors to default */
PROCEDURE(RESETCOLORS) {
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 7); // Light gray on black
#else
    printf("\033[0m"); // Reset all attributes
    fflush(stdout);
#endif
    
    RETURNINTX(0);
ENDPROC
}

/* SCREENSIZE - Get screen dimensions */
PROCEDURE(SCREENSIZE) {
    int width = 80, height = 25; // Default values
    
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            
            // Sanity check - ensure positive values
            if (width <= 0) width = 80;
            if (height <= 0) height = 25;
        }
    }
#else
    // For Unix-like systems, we need to include sys/ioctl.h
    #include <sys/ioctl.h>
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        width = w.ws_col;
        height = w.ws_row;
        
        // Sanity check - ensure positive values
        if (width <= 0) width = 80;
        if (height <= 0) height = 25;
    }
#endif
    
    // Return width and height as a string "width height"
    char result[32];
    sprintf(result, "%d %d", width, height);
    RETURNSTRX(result);
ENDPROC
}

/* PRINTAT - Print text at specific position */
PROCEDURE(PRINTAT) {
    int row = 0, col = 0;
    char* text;

    row = GETINT(ARG0);
    col = GETINT(ARG1);
    text = GETSTRING(ARG2);
    
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {col, row};
    SetConsoleCursorPosition(hConsole, pos);
    printf("%s", text);
#else
    printf("\033[%d;%dH%s", row+1, col+1, text);
    fflush(stdout);
#endif
    
    RETURNINTX(0);
ENDPROC
}

PROCEDURE(WAIT) {
    int sleep = GETINT(ARG0);
    wait(sleep);
    RETURNINT(0);
}

/* OPENCONSOLE - Open a new console window */
PROCEDURE(OPENCONSOLE) {
    char* title = "CREXX Console";
    int width = 80;
    int height = 25;
    
    // Get optional parameters
    title = GETSTRING(ARG0);
    width = GETINT(ARG1);
    height = GETINT(ARG2);
#ifdef _WIN32
    // First, try to get the current console
    HWND consoleWnd = GetConsoleWindow();
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    
    // If we don't have valid handles, try to create a new console
    if (hStdOut == INVALID_HANDLE_VALUE || hStdIn == INVALID_HANDLE_VALUE) {
        // Try to detach from any existing console first
        FreeConsole();
        
        // Then allocate a new one
        if (!AllocConsole()) {
            // If allocation fails, return error
            RETURNINTX(-8);
        }
        
        // Get the new handles
        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        hStdIn = GetStdHandle(STD_INPUT_HANDLE);
        
        // Redirect standard I/O
        freopen("CONOUT$", "w", stdout);
        freopen("CONIN$", "r", stdin);
    }
    
    // Set console title
    SetConsoleTitle(title);
    
    // Set console buffer size - make sure width/height are reasonable
    if (width < 10) width = 10;
    if (height < 10) height = 10;
    if (width > 200) width = 200;
    if (height > 200) height = 200;
    
    COORD bufferSize = {width, height};
    SetConsoleScreenBufferSize(hStdOut, bufferSize);
    
    // Set console window size
    SMALL_RECT windowRect = {0, 0, width-1, height-1};
    SetConsoleWindowInfo(hStdOut, TRUE, &windowRect);
    
    // Make sure the console window is visible
    consoleWnd = GetConsoleWindow();
    if (consoleWnd != NULL) {
        ShowWindow(consoleWnd, SW_SHOW);
        SetForegroundWindow(consoleWnd);
    }
    
    // Clear the screen to start fresh
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    COORD homeCoords = {0, 0};
    
    if (GetConsoleScreenBufferInfo(hStdOut, &csbi)) {
        DWORD cellCount = csbi.dwSize.X * csbi.dwSize.Y;
        FillConsoleOutputCharacter(hStdOut, ' ', cellCount, homeCoords, &count);
        FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count);
        SetConsoleCursorPosition(hStdOut, homeCoords);
    }
    
    RETURNINTX(0); // Success
#else
    // For Unix-like systems, we can use xterm or similar terminal emulator
    char command[256];
    sprintf(command, "xterm -title \"%s\" -geometry %dx%d -e \"$SHELL\" &", 
            title, width, height);
    int result = system(command);
    
    RETURNINTX(result == 0 ? 0 : -8);
#endif
ENDPROC
}

/* CLOSECONSOLE - Close the console window */
PROCEDURE(CLOSECONSOLE) {
#ifdef _WIN32
    // For Windows, we can use FreeConsole
    RETURNINTX(FreeConsole() ? 1 : 0);
#else
    // For Unix-like systems, this is more complex
    // We'd need to know which terminal we opened
    RETURNINTX(0);
#endif
ENDPROC
}
/* NEWCONSOLE - Create a new dedicated console window */
PROCEDURE(NEWCONSOLE) {
    char* title = GETSTRING(ARG0);
    int width = GETINT(ARG1);
    int height = GETINT(ARG2);
    
#ifdef _WIN32
    // Detach from current console
    FreeConsole();
    
    // Create a new console
    if (!AllocConsole()) {
        RETURNINTX(-1);
    }
    
    // Set console title
    SetConsoleTitle(title);
    
    // Get standard handles
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
    
    // Redirect standard I/O
    FILE *fpstdin, *fpstdout, *fpstderr;
    
    // Redirect STDIN
    freopen_s(&fpstdin, "CONIN$", "r", stdin);
    
    // Redirect STDOUT
    freopen_s(&fpstdout, "CONOUT$", "w", stdout);
    
    // Redirect STDERR
    freopen_s(&fpstderr, "CONOUT$", "w", stderr);
    
    // Set buffer size
    if (width < 10) width = 80;
    if (height < 10) height = 25;
    
    COORD bufferSize = {width, height};
    SetConsoleScreenBufferSize(hStdOut, bufferSize);
    
    // Set window size
    SMALL_RECT windowRect = {0, 0, width-1, height-1};
    SetConsoleWindowInfo(hStdOut, TRUE, &windowRect);
    
    // Make console window visible and bring to front
    HWND consoleWnd = GetConsoleWindow();
    ShowWindow(consoleWnd, SW_SHOW);
    SetForegroundWindow(consoleWnd);
    
    // Clear the screen
    system("cls");
    
    RETURNINTX(0);
#else
    // For Unix systems, we'll launch a new terminal with our program
    char command[512];
    char selfPath[256];
    
    // Get path to current executable
    if (readlink("/proc/self/exe", selfPath, sizeof(selfPath)-1) == -1) {
        strcpy(selfPath, "crexx"); // Fallback
    }
    
    // Create command to launch a new terminal with our program
    sprintf(command, "xterm -title \"%s\" -geometry %dx%d -e \"%s --console\" &", 
            title, width, height, selfPath);
    
    int result = system(command);
    RETURNINTX(result == 0 ? 0 : -1);
#endif
ENDPROC
}

/* CONSOLE_INPUT - Get input from the dedicated console */
PROCEDURE(CONSOLE_INPUT) {
    char buffer[1024] = {0};
    char* prompt = GETSTRING(ARG0);
    int maxLen = GETINT(ARG1);
    
    if (maxLen <= 0 || maxLen > 1023) maxLen = 1023;
    
    // Display prompt
    printf("%s", prompt);
    fflush(stdout);
    
    // Get input
    if (fgets(buffer, maxLen, stdin) != NULL) {
        // Remove trailing newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
    }
    
    RETURNSTRX(buffer);
ENDPROC
}

/* CONSOLE_GETCHAR - Get a single character from the dedicated console */
PROCEDURE(CONSOLE_GETCHAR) {
    int echo = GETINT(ARG0);  // Whether to echo the character
    int ch;
    
#ifdef _WIN32
    // Windows implementation
    if (echo) {
        ch = _getch();
        printf("%c", ch);
        fflush(stdout);
    } else {
        ch = _getch();
    }
#else
    // Unix implementation
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    
    if (!echo) {
        newt.c_lflag &= ~(ICANON | ECHO);
    } else {
        newt.c_lflag &= ~ICANON;
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    
    RETURNINTX(ch);
ENDPROC
}

/* EXTENDEDKEY - Get extended key input including function keys */
PROCEDURE(EXTENDEDKEY) {
    int key = 0;
    int extended = 0;
    
#ifdef _WIN32
    // Windows implementation
    key = _getch();
    
    // Check if it's an extended key (returns 0 or 0xE0 first)
    if (key == 0 || key == 0xE0) {
        extended = 1;
        key = _getch();  // Get the actual key code
    }
    
    // Map common extended keys to consistent values across platforms
    if (extended) {
        switch (key) {
            // Function keys
            case 59: key = 1001; break;  // F1
            case 60: key = 1002; break;  // F2
            case 61: key = 1003; break;  // F3
            case 62: key = 1004; break;  // F4
            case 63: key = 1005; break;  // F5
            case 64: key = 1006; break;  // F6
            case 65: key = 1007; break;  // F7
            case 66: key = 1008; break;  // F8
            case 67: key = 1009; break;  // F9
            case 68: key = 1010; break;  // F10
            case 133: key = 1011; break; // F11
            case 134: key = 1012; break; // F12
            
            // Arrow keys
            case 72: key = 1101; break;  // Up arrow
            case 80: key = 1102; break;  // Down arrow
            case 75: key = 1103; break;  // Left arrow
            case 77: key = 1104; break;  // Right arrow
            
            // Other navigation keys
            case 71: key = 1201; break;  // Home
            case 79: key = 1202; break;  // End
            case 73: key = 1203; break;  // Page Up
            case 81: key = 1204; break;  // Page Down
            case 82: key = 1205; break;  // Insert
            case 83: key = 1206; break;  // Delete
            
            // Add more mappings as needed
        }
    }
#else
    // Unix implementation using escape sequences
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    // Read first character
    key = getchar();
    
    // Check for escape sequence
    if (key == 27) {
        // Set a short timeout to check if it's a standalone ESC key
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000;  // 100ms timeout
        
        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv) > 0) {
            // It's an escape sequence
            key = getchar();
            
            if (key == '[') {
                // Most common escape sequences start with ESC[
                key = getchar();
                
                // Map to the same values as Windows
                switch (key) {
                    // Arrow keys
                    case 'A': key = 1101; break;  // Up arrow
                    case 'B': key = 1102; break;  // Down arrow
                    case 'D': key = 1103; break;  // Left arrow
                    case 'C': key = 1104; break;  // Right arrow
                    
                    // Function keys and others often have more complex sequences
                    case '1': 
                        key = getchar();  // Get next char
                        if (key == '~') key = 1201;  // Home
                        else if (key == '1') {
                            key = getchar();  // Get next char
                            if (key == '~') key = 1011;  // F11
                        }
                        else if (key == '2') {
                            key = getchar();  // Get next char
                            if (key == '~') key = 1012;  // F12
                        }
                        break;
                    case '2': 
                        key = getchar();  // Get next char
                        if (key == '~') key = 1205;  // Insert
                        break;
                    case '3': 
                        key = getchar();  // Get next char
                        if (key == '~') key = 1206;  // Delete
                        break;
                    case '4': 
                        key = getchar();  // Get next char
                        if (key == '~') key = 1202;  // End
                        break;
                    case '5': 
                        key = getchar();  // Get next char
                        if (key == '~') key = 1203;  // Page Up
                        break;
                    case '6': 
                        key = getchar();  // Get next char
                        if (key == '~') key = 1204;  // Page Down
                        break;
                }
            } else if (key == 'O') {
                // Some function keys use ESC O sequence
                key = getchar();
                
                // Map function keys
                switch (key) {
                    case 'P': key = 1001; break;  // F1
                    case 'Q': key = 1002; break;  // F2
                    case 'R': key = 1003; break;  // F3
                    case 'S': key = 1004; break;  // F4
                    // F5-F12 often use different sequences
                }
            }
        } else {
            // It's a standalone ESC key
            key = 27;
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    RETURNINTX(key);
ENDPROC
}

/* KEYNAME - Get the name of a key from its code */
PROCEDURE(KEYNAME) {
    int keyCode = GETINT(ARG0);
    char* keyName = "Unknown";
    
    // Regular ASCII keys
    if (keyCode > 32 && keyCode <= 126) {
        static char asciiName[2];
        asciiName[0] = (char)keyCode;
        asciiName[1] = '\0';
        RETURNSTRX(asciiName);
    }
    
    // Control keys and extended keys
    switch (keyCode) {
        // Control keys
        case 0: keyName = "Null"; break;
        case 8: keyName = "Backspace"; break;
        case 9: keyName = "Tab"; break;
        case 10: case 13: keyName = "Enter"; break;
        case 27: keyName = "Escape"; break;
        case 32: keyName = "SpaceBar"; break;
        
        // Extended keys
        case 1001: keyName = "F1"; break;
        case 1002: keyName = "F2"; break;
        case 1003: keyName = "F3"; break;
        case 1004: keyName = "F4"; break;
        case 1005: keyName = "F5"; break;
        case 1006: keyName = "F6"; break;
        case 1007: keyName = "F7"; break;
        case 1008: keyName = "F8"; break;
        case 1009: keyName = "F9"; break;
        case 1010: keyName = "F10"; break;
        case 1011: keyName = "F11"; break;
        case 1012: keyName = "F12"; break;
        
        // Arrow keys
        case 1101: keyName = "Up"; break;
        case 1102: keyName = "Down"; break;
        case 1103: keyName = "Left"; break;
        case 1104: keyName = "Right"; break;
        
        // Navigation keys
        case 1201: keyName = "Home"; break;
        case 1202: keyName = "End"; break;
        case 1203: keyName = "Page Up"; break;
        case 1204: keyName = "Page Down"; break;
        case 1205: keyName = "Insert"; break;
        case 1206: keyName = "Delete"; break;
        
        default: keyName = "Unknown"; break;
    }
    
    RETURNSTRX(keyName);
ENDPROC
}

/* FULLCLEAR - Completely clear the console and reset state */
PROCEDURE(FULLCLEAR) {
#ifdef _WIN32
    // Get console handle
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if (hConsole != INVALID_HANDLE_VALUE) {
        // Get current console info
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        
        // Get console size
        DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
        
        // Fill console with spaces
        DWORD charsWritten;
        COORD topLeft = {0, 0};
        FillConsoleOutputCharacter(hConsole, ' ', consoleSize, topLeft, &charsWritten);
        
        // Reset attributes
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, consoleSize, topLeft, &charsWritten);
        
        // Move cursor to top left
        SetConsoleCursorPosition(hConsole, topLeft);
        
        // Reset text attributes
        SetConsoleTextAttribute(hConsole, 0x07);  // Light gray on black
    }
    
    // Use system command as well for thoroughness
    system("cls");
#else
    // For Unix systems
    // Reset all attributes and clear screen
    printf("\033[0m\033[2J\033[3J\033[H");
    fflush(stdout);
    
    // Use system command as well
    system("clear");
#endif

    RETURNINTX(0);
ENDPROC
}
PROCEDURE(STRIM){
    char *str=GETSTRING(ARG0) ;
    char *end;
 // Trim leading spaces
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) RETURNSTRX("")// If the string is all spaces
 // Trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
 // Null terminate the string
    *(end + 1) = '\0';
    RETURNSTRX(str);
ENDPROC
}

LOADFUNCS
    ADDPROC(GETCHAR, "console.getchar", "b", ".int", "timeout=.int");
    ADDPROC(READSTRING, "console.readconsole", "b", ".string", "maxlen=.int,prompt=.string");
    ADDPROC(KBHIT, "console.kbhit", "b", ".int", "");
    ADDPROC(GETKEY, "console.getkey", "b", ".int", "");
    ADDPROC(SETCURSOR, "console.setcursor", "b", ".int", "row=.int,col=.int");
    ADDPROC(CLRSCR, "console.clrscr", "b", ".int", "");
    ADDPROC(SETCOLOR, "console.setcolor", "b", ".int", "fgcolor=.int,bgcolor=.int");
    ADDPROC(RESETCOLORS, "console.resetcolors", "b", ".int", "");
    ADDPROC(SCREENSIZE, "console.screensize", "b", ".string","");
    ADDPROC(PRINTAT, "console.printat", "b", ".int", "row=.int,col=.int,string=.string");
    ADDPROC(WAIT, "console.wait", "b", ".int", "sleep=.int");
    ADDPROC(OPENCONSOLE, "console.openconsole", "b", ".int", "title=.string,width=.int,height=.int");
    ADDPROC(CLOSECONSOLE, "console.closeconsole", "b", ".int", "");
    ADDPROC(NEWCONSOLE, "console.newconsole", "b", ".int", "title=.string,width=.int,height=.int");
    ADDPROC(CONSOLE_INPUT, "console.console_input", "b", ".string", "prompt=.string,maxlen=.int");
    ADDPROC(CONSOLE_GETCHAR, "console.console_getchar", "b", ".int", "echo=.int");
    ADDPROC(SETCOLOR, "console.setcolor", "b", ".int", "fgcolor=.int,bgcolor=.int");
    ADDPROC(EXTENDEDKEY, "console.extendedkey", "b", ".int", "");
    ADDPROC(KEYNAME, "console.keyname", "b", ".string", "keycode=.int");
    ADDPROC(FULLCLEAR, "console.fullclear", "b", ".int", "");
    ADDPROC(STRIM, "console.strim", "b", ".string", "string=.string");
    ADDPROC(STRIM, "console.mydummy", "b", ".string", "string=.string");
ENDLOADFUNCS
