# CREXX Console Functions

This document describes the functions available in the `regscreen` plugin for CREXX, which provides console and terminal handling capabilities.


## Console Management

### newconsole(title, width, height)

Creates a new dedicated console window that is fully controlled by your program.

**Parameters:**
- `title` - Title for the console window
- `width` - Width of the console in characters
- `height` - Height of the console in characters

**Returns:** 0 on success, negative value on failure

**Example:**
```rexx
/* Create a new console window */
result = newconsole("My Application", 80, 25)
IF result < 0 THEN DO
   SAY "Failed to create console window"
   EXIT
END
```

### openconsole(title, width, height)

Opens or configures an existing console window.

**Parameters:**
- `title` - Title for the console window
- `width` - Width of the console in characters
- `height` - Height of the console in characters

**Returns:** 0 on success, negative value on failure

### closeconsole()

Closes the current console window.

**Returns:** 0 on success, negative value on failure

## Screen Clearing Functions

### clrscr()

Clears the screen content and moves the cursor to the home position (0,0).
This is a lightweight screen clearing operation suitable for routine use.

**Example:**
```rexx
/* Clear the screen */
CALL clrscr
```

### fullclear()

Performs a thorough screen clearing operation that:
- Clears all content from the screen
- Resets text and background colors to defaults
- Moves the cursor to the home position
- Ensures the entire buffer is reset

Use this function when you need a complete reset of the console state,
such as at application startup/shutdown or when display issues occur.

**Example:**
```rexx
/* Completely reset the console */
CALL fullclear
```

## Cursor and Text Positioning

### setcursor(row, col)

Positions the cursor at the specified row and column.

**Parameters:**
- `row` - Row position (0-based)
- `col` - Column position (0-based)

**Example:**
```rexx
/* Move cursor to row 5, column 10 */
CALL setcursor 5, 10
```

### printat(row, col, text)

Prints text at the specified position.

**Parameters:**
- `row` - Row position (0-based)
- `col` - Column position (0-based)
- `text` - Text to display

**Example:**
```rexx
/* Print a message at row 10, column 5 */
CALL printat 10, 5, "Hello, World!"
```

### screensize()

Gets the current console screen dimensions.

**Returns:** A string containing width and height separated by a space

**Example:**
```rexx
/* Get screen dimensions */
size = screensize()
PARSE VAR size width height
SAY "Screen is" width "columns by" height "rows"
```

## Color Functions

### setcolor(fgcolor, bgcolor)

Sets both text (foreground) and background colors in a single call.

**Parameters:**
- `fgcolor` - Foreground color (text color) from 0-15, or -1 to leave unchanged
- `bgcolor` - Background color from 0-15, or -1 to leave unchanged

**Example:**
```rexx
/* Set yellow text on blue background */
CALL setcolor 14, 1

/* Change just the text color to green, leave background unchanged */
CALL setcolor 2, -1

/* Change just the background to red, leave text color unchanged */
CALL setcolor -1, 4
```

### resetcolors()

Resets colors to default (usually light gray on black).

**Example:**
```rexx
/* Reset colors to default */
CALL resetcolors
```

**Color Codes:**
- 0: Black
- 1: Blue
- 2: Green
- 3: Cyan
- 4: Red
- 5: Magenta
- 6: Brown/Yellow
- 7: Light Gray
- 8: Dark Gray
- 9: Light Blue
- 10: Light Green
- 11: Light Cyan
- 12: Light Red
- 13: Light Magenta
- 14: Yellow
- 15: White

## Input Functions

### getchar(timeout)

Gets a single character from the keyboard without echoing it to the screen.

**Parameters:**
- `timeout` - Timeout in milliseconds, or 0/negative for no timeout

**Returns:** ASCII code of the character pressed, or 0 if timeout occurred

**Example:**
```rexx
/* Wait for a key press with 5 second timeout */
key = getchar(5000)
IF key = 0 THEN
   SAY "No key pressed within timeout"
ELSE
   SAY "You pressed key with ASCII code:" key
```

### console_getchar(echo)

Gets a single character from the dedicated console.

**Parameters:**
- `echo` - Whether to echo the character (1) or not (0)

**Returns:** ASCII code of the character pressed

**Example:**
```rexx
/* Get a key without echoing it */
key = console_getchar(0)
```

### extendedkey()

Gets extended key input including function keys, arrow keys, and other special keys.

**Returns:** 
- For regular keys: ASCII code (0-255)
- For function keys: 1001-1012 (F1-F12)
- For arrow keys: 1101-1104 (Up, Down, Left, Right)
- For navigation keys: 1201-1206 (Home, End, PgUp, PgDn, Insert, Delete)

**Example:**
```rexx
/* Get a key, including special keys */
key = extendedkey()
keyName = keyname(key)
SAY "You pressed:" keyName
```

### keyname(keycode)

Gets the name of a key from its code.

**Parameters:**
- `keycode` - Key code from extendedkey()

**Returns:** String name of the key

**Example:**
```rexx
key = extendedkey()
SAY "You pressed the" keyname(key) "key"
```

### kbhit()

Checks if a key has been pressed without reading it.

**Returns:** 1 if a key is available, 0 otherwise

**Example:**
```rexx
/* Check if a key is available */
IF kbhit() THEN
   key = getchar(0)
ELSE
   SAY "No key pressed yet"
```

### readconsole(maxlen, prompt)

Reads a string from the console with the specified prompt.

**Parameters:**
- `maxlen` - Maximum length of input
- `prompt` - Prompt string to display

**Returns:** The string entered by the user

**Example:**
```rexx
/* Read user input with a prompt */
name = readconsole(30, "Enter your name: ")
SAY "Hello," name
```

### console_input(prompt, maxlen)

Reads a string from the dedicated console.

**Parameters:**
- `prompt` - Prompt string to display
- `maxlen` - Maximum length of input

**Returns:** The string entered by the user

**Example:**
```rexx
/* Read user input from dedicated console */
input = console_input("Enter command: ", 50)
```

## Utility Functions

### wait(milliseconds)

Pauses execution for the specified number of milliseconds.

**Parameters:**
- `milliseconds` - Time to wait in milliseconds

**Example:**
```rexx
/* Wait for 2 seconds */
CALL wait 2000
```

## Example Programs

### Basic Console Demo

```rexx
/* Initialize a new console window */
result = newconsole("CREXX Console Demo", 80, 25)
IF result < 0 THEN EXIT

/* Clear the screen */
CALL fullclear

/* Set colors and display a title */
CALL setcolor 15, 1  /* White text on blue background */
CALL printat 2, 20, "CREXX Console Demo"
CALL resetcolors

/* Display some information */
CALL printat 5, 5, "This is a demonstration of the regscreen console functions."
CALL printat 7, 5, "Press any key to continue..."

/* Wait for a keypress */
dummy = console_getchar(0)

/* Clean up */
CALL fullclear
EXIT
```

### Extended Key Test

```rexx
/* Initialize a new console window */
result = newconsole("Extended Key Test", 80, 25)
IF result < 0 THEN EXIT

/* Clear the screen */
CALL fullclear

/* Display instructions */
CALL setcolor 14, -1  /* Yellow text */
CALL printat 2, 5, "Press any key to see its code and name (ESC to exit)"
CALL resetcolors

/* Key test loop */
row = 5
DO UNTIL key = 27  /* ESC key */
   /* Get a key */
   key = extendedkey()
   keyName = keyname(key)
   
   /* Display key information */
   CALL printat row, 5, "Key code: " || RIGHT(key, 5) || "   Key name: " || keyName
   
   /* Move to next row, wrapping if needed */
   row = row + 1
   IF row > 20 THEN row = 5
END

/* Clean up */
CALL fullclear
EXIT
```

### Color Demo

```rexx
/* Initialize console */
CALL newconsole("Color Demo", 80, 25)
CALL fullclear

/* Show all foreground colors */
CALL printat 2, 5, "Foreground Colors:"
DO color = 0 TO 15
   CALL setcolor color, -1
   CALL printat 3, 5 + color * 4, color
END

/* Show all background colors */
CALL printat 5, 5, "Background Colors:"
DO color = 0 TO 7
   CALL setcolor 15, color
   CALL printat 6, 5 + color * 8, "BG:" || color
END

/* Reset colors and exit */
CALL resetcolors
CALL printat 8, 5, "Press any key to exit..."
dummy = getchar(0)
CALL fullclear
EXIT
```

## Operating System Support

### Windows
The console module is fully supported on Windows operating systems. It utilizes Windows-specific APIs for console manipulation, such as setting cursor positions, clearing the screen, and handling keyboard input. Users can expect seamless integration with Windows console features, including support for colored text output and console window management.

### macOS
On macOS, the console module leverages Unix-like behavior for console operations. It uses terminal control sequences for cursor positioning and screen clearing. Users can interact with the console in a manner similar to Linux, with support for non-blocking input and customizable text attributes.

### Linux
The console module is designed to work on Linux systems, utilizing standard Unix terminal features. It supports ANSI escape codes for text formatting and cursor control. The module also includes functionality for reading input in a non-blocking manner, making it suitable for interactive applications. Users can expect consistent behavior across various Linux distributions.