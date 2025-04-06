options levelb
import console
import rxfnsb

/* Test extended key input */
call mydummy " Fred"
say substr("Mary",2,2)
background=15
width=100
height=32
result = newconsole("ISPF like Selection Menu", width,height)
IF result < 0 THEN DO
   SAY "Failed to create console window"
   EXIT
END

call fillBackground width,height,background
call clrscr

call setcolor 15, 5          /* Set foreground to white and background to blue */
call printat  2, 10, "Welcome to the ISPF-like Menu"
call setcolor 0, background   /* Yellow text on default background */
call printat 3, 10, "=============================="

do forever
    /* Display menu options with colors */
    call setcolor 5, background   /* Yellow text on default background */
    call printat 7, 5, "1   Option 1 - Perform Action 1"
    call printat 8, 5, "2   Option 2 - Perform Action 2"
    call printat 9, 5, "3   Option 3 - Perform Action 3"
    call setcolor 4, background   /* Reset colors for prompt */
    call printat 10, 5,"4   Exit"
    call setcolor 1, background   /* Reset colors for prompt */
    call printat 12, 5, "Please select an option (1-4):                                              "
    CALL setcursor 12, 36
    /* Get user input */
    userInput = strim(readconsole(16,""))  /* 0 = no echo */
    call printat height, 0, copies(" ",width)    ## clear message line
     if STrim(userInput)="" then iterate

    /* Process user input */
      if  userInput = "1" then do
            call printat height, 5, "You selected Option 1."
            /* Call function or perform action for Option 1 */
      end
      else if  userInput = "2" then do
            call printat height, 5, "You selected Option 2."
            /* Call function or perform action for Option 2 */
      end
      else if  userInput = "3" then do
            call printat height, 5, "You selected Option 3."
            /* Call function or perform action for Option 3 */
      end
      else if  userInput = "4" then do
            call printat height, 5, "Exiting the menu."
            exit
      end
      else do
           call printat height, 5, "Invalid selection: '"userInput"'. Please try again."
      end

    /* Clear the input line */
end
exit


/* Initialize a new console window */
result = newconsole("Extended Key Test", 80, 25)
IF result < 0 THEN DO
   SAY "Failed to create console window"
   EXIT
END

/* Clear the screen */
CALL clrscr

/* Display instructions */
CALL setcolor 15, 1  /* White on blue */
CALL printat 2, 10, "Extended Key Test"
CALL resetcolors

CALL printat 4, 5, "Press any key, including function keys, arrow keys, etc."
CALL printat 5, 5, "Press ESC three times to exit"

/* Key test loop */
row = 7
escCount = 0

DO WHILE escCount < 3
   /* Get a key */
   CALL printat row, 5, "Press a key: "
   key = extendedkey()
   
   /* Get key name */
   keyName = keyname(key)
   
   /* Display key information */
   CALL printat row, 20, "Key code: " || RIGHT(key, 5) || "   Key name: " || keyName
   
   /* Check for ESC key */
   IF key = 27 THEN DO
      escCount = escCount + 1
      CALL printat row, 60, "ESC count: " || escCount
   end
   ELSE escCount = 0
   
   /* Move to next row, wrapping if needed */
   row = row + 1
   IF row > 20 THEN DO
      /* Scroll the display up */
      DO i = 7 TO 20
         line = ""
         DO j = 5 TO 70
            CALL setcursor i+1, j
            ch = getchar(0)
            line = line || ch
         END
         CALL printat i, 5, line
      END
      row = 20
   END
END

/* Clean up */
CALL fullclear
CALL printat 10, 20, "Test complete!"
CALL wait 1000



/* Example REXX script demonstrating console functions */
/* Test dedicated console window */
CALL fullclear

/* Clear the screen and set colors */

/* Display welcome message */
CALL printat 2, 10, "Welcome to the CREXX Dedicated Console!"
CALL printat 4, 10, "This console is fully controlled by your program."

/* Test character input */
CALL printat 6, 10, "Press any key (will not echo)..."
key = console_getchar(0)  /* 0 = no echo */
CALL printat 8, 10, "You pressed key with ASCII code: " || key

/* Test string input */
CALL printat 10, 10, "Now let's test string input:"
instr = console_input("Enter some text: ", 50)
CALL printat 12, 10, "You entered: " || instr

/* Wait for user to press a key before exiting */
CALL printat 14, 10, "Press any key to exit..."
dummy = console_getchar(0)

/* Clean up */
CALL clrscr

/* CREXX Color Demo - Demonstrates the setcolor function */

/* Initialize console */
CALL clrscr

/* Basic color examples */
CALL printat 2, 5, "Basic Color Examples:"

/* Set just foreground color (yellow text) */
CALL setcolor 14, -1
CALL printat 4, 5, "This text has yellow foreground, unchanged background"

/* Set just background color (blue background) */
CALL setcolor -1, 1
CALL printat 6, 5, "This text has unchanged foreground, blue background"

/* Set both colors (white on red) */
CALL setcolor 15, 4
CALL printat 8, 5, "This text has white foreground, red background"

/* Reset colors */
CALL resetcolors
CALL printat 10, 5, "Colors have been reset to default"

/* Color table */
CALL printat 12, 5, "Color Table:"

row = 14
DO fg = 0 TO 15
   /* Show color number and name */
   CALL setcolor fg, -1
   colorName = GetColorName(fg)
   CALL printat row, 5, RIGHT(fg, 2) || ": " || colorName

   /* Show the same color on different backgrounds */
   DO bg = 0 TO 3
      CALL setcolor fg, bg
      CALL printat row, 30 + bg * 10, "Sample"
   END

   row = row + 1
END

/* Reset and exit */
CALL resetcolors
CALL printat row + 2, 5, "Press any key to exit..."
key = getchar(0)
CALL clrscr
EXIT

/* Get color name based on code */
GetColorName: PROCEDURE=.string
   arg code=.int
      if  code = 0 THEN RETURN "Black"
      if  code = 1 THEN RETURN "Blue"
      if  code = 2 THEN RETURN "Green"
      if  code = 3 THEN RETURN "Cyan"
      if  code = 4 THEN RETURN "Red"
      if  code = 5 THEN RETURN "Magenta"
      if  code = 6 THEN RETURN "Brown"
      if  code = 7 THEN RETURN "Light Gray"
      if  code = 8 THEN RETURN "Dark Gray"
      if  code = 9 THEN RETURN "Light Blue"
      if  code = 10 THEN RETURN "Light Green"
      if  code = 11 THEN RETURN "Light Cyan"
      if  code = 12 THEN RETURN "Light Red"
      if  code = 13 THEN RETURN "Light Magenta"
      if  code = 14 THEN RETURN "Yellow"
      if  code = 15 THEN RETURN "White"
return "Unknown"

fillBackground: procedure
    arg width=.int,rows=int,background=.int
    /* Set the desired background color */
    call setcolor 1, background     /* Example: Black text on Blue background */
    /* Fill the console with spaces to simulate background color */
    fill=copies(" ",width)
    do row = 0 to rows  /* Adjust based on your console height */
       call printat row, 0,fill     /* Adjust width as needed */
    end
return