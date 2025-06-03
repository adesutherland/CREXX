# GUI Programming

## Overview

This chapter provides an overview of a lightweight graphical user interface (GUI) plugin for \crexx{}, focusing on the most essential widgets. 
<!-- It contains two files: `gui.c`, which implements the GUI functionality using GTK, and `gui_test.rexx`, which demonstrates how to use the procedures defined in `gui.c`. -->

## Quick Start Guide

### Basic Window Creation

Creating a basic window is the first step in building a GUI application. Here's a minimal example:

```rexx <!--initwindow-->
/* Initialize the window */
call init_window "My First GUI", 400, 300  /* title, width, height */

/* Show the window */
call show_window

/* Process events */
do forever
    event = process_events(500)  /* Check events every 500ms */
    if event < 0 then leave      /* Exit if window is closed */
end
```

### Adding Common Widgets

Here's how to add basic widgets to your window:

```rexx <!--basicwidgets.rexx-->
/* Add a label */
text1 = add_text("Enter your name:", 10, 10)

/* Add an input field */
input1 = add_edit(10, 40, 200)

/* Add a button */
button1 = add_button("Submit", 10, 70)

/* Add a list */
list1 = add_list(10, 100, 200, 100)  /* x, y, width, height */
call list_add_item list1, "Item 1", "lightblue"
call list_add_item list1, "Item 2", "lightgreen"

/* Add a combo box */
items.1 = "Option 1"
items.2 = "Option 2"
items.3 = "Option 3"
combo1 = add_combo(items, 10, 220)
```

### Complete Working Example

Here's a complete example that demonstrates common GUI patterns:

```rexx <!--completeGuiExample.rexx-->
/* Initialize GUI */
call init_window "Contact Form", 300, 400

/* Add form fields */
call add_text "Name:", 10, 10
name_field = add_edit(10, 30, "")

call add_text "Email:", 10, 70
email_field = add_edit(10, 90, "")

call add_text "Type:", 10, 130
type.1 = "Personal"
type.2 = "Business"
type.3 = "Other"
type_combo = add_combo(type, 10, 150)

/* Add buttons */
submit = add_button("Submit", 10, 200)
clear = add_button("Clear", 120, 200)

/* Add status area */
status = add_text("Ready", 10, 350)

/* Show window */
call show_window

/* Main event loop */
do forever
    event = process_events(500)
    
    if event < 0 then leave              /* Window closed */
    else if event = submit then do       /* Submit button clicked */
        name = get_widget_address(name_field)
        email = get_widget_address(email_field)
        if name = "" | email = "" then
            call set_text status, "Please fill all fields"
        else
            call set_text status, "Form submitted!"
    end
    else if event = clear then do        /* Clear button clicked */
        call set_text name_field, ""
        call set_text email_field, ""
        call set_text status, "Form cleared"
    end
end
```

### Common Patterns and Best Practices

#### GUI Layout Guidelines

1. **Widget Spacing**
   - Leave 10-20 pixels between widgets
   - Group related widgets together
   - Align widgets for visual consistency

2. **Layout Organization**
   - Place important controls at the top
   - Group related controls together
   - Use consistent spacing and alignment
   - Consider tab order for input fields

3. **Size Guidelines**
   - Make clickable elements at least 25x25 pixels
   - Leave enough space for text in labels
   - Consider window resizing behavior

#### Error Handling Strategies

1. **Input Validation**
```rexx <!--validation.rexx-->
/* Validate required fields */
if input_text = "" then do
    call set_status status_bar, "Please fill required field"
    call set_sensitive submit_button, 0
    return
end
```

2. **Widget Creation**
```rexx <!--widgetcreation.rexx-->
button = add_button("Click Me", 10, 10)
if button < 0 then do
    call notify_pick "Error", "Failed to create button", "error"
    return
end
```

3. **File Operations**
```rexx <!--fileroperations.rexx-->
rc = copy_file(source, target)
if rc \= 0 then do
    call notify_pick "Error", "Failed to copy file", "error"
    return
end
```

#### Common Usage Patterns

1. **Enable/Disable Controls**
```rexx <!--enablecontrols.rexx-->
/* Disable submit until valid */
call set_sensitive submit_button, 0

/* Enable when valid */
if valid_input then
    call set_sensitive submit_button, 1
```

2. **Status Updates**
```rexx <!--statusupdates-->
/* Show operation status */
call set_status status_bar, "Processing..."
/* ... do work ... */
call set_status status_bar, "Complete"
```

3. **Progress Feedback**
```rexx <!--progress.rexx-->
/* Show splash during long operation */
call splash_pick "Working", "Please wait...", 0, 300, 100
/* ... do work ... */
call cleanup_gui
```

#### Performance Tips

1. **Event Processing**
   - Use appropriate timeout values in process_events
   - Avoid lengthy operations in event loop
   - Consider using background processing for long tasks

2. **Memory Management**
   - Clean up resources when no longer needed
   - Use cleanup_gui when closing application
   - Free any allocated memory

3. **Widget Updates**
   - Batch multiple updates together
   - Avoid unnecessary widget refreshes
   - Use efficient data structures

---

## Installation Instructions

To set up the environment for running the GUI application, follow these steps:

### For Linux
1. **Install GTK**: Ensure your system has GTK installed. You can install it using your package manager. For example:
   - On Ubuntu/Debian:
```bash <!--installubuntu.sh-->
     sudo apt-get install libgtk-3-dev
```
   - On Fedora:
```bash <!--installfedora.sh-->
     sudo dnf install gtk3-devel
 ```
   - On macOS using Homebrew:
```bash <!--installmacos.sh-->
     brew install gtk+3
```

2. **Compile the Code**: Use a C compiler to compile `gui.c`. For example:
```bash <!--compilecode.sh-->
   gcc -o gui gui.c `pkg-config --cflags --libs gtk+-3.0`
```

### For Windows

1. **Install GTK**: Download the GTK installer for Windows from the [GTK website](https://www.gtk.org/download/windows.php). Follow the instructions to install GTK on your system.

2. **Set Environment Variables**: After installation, you may need to set the `PATH` environment variable to include the GTK `bin` directory. This allows you to run GTK applications from the command line.

3. **Compile the Code**: Use a C compiler like MinGW or MSYS2 to compile `gui.c`. Open a terminal and run:
```bash <!--compilewindows.sh-->
   gcc -o gui gui.c `pkg-config --cflags --libs gtk+-3.0`
```

---

## CREXX GUI Documentation

### Function Quick Reference

### Window and Widget Functions

#### INIT_WINDOW

```rexx <!--initwindow.rexx-->
init_window(title,width,height)(title,width,height)
```

- **Description**: Initializes the main application window and sets up the GTK environment.
- **Parameters**:
  - `title`: Title of the window.
  - `width`: Width of the window.
  - `height`: Height of the window.
- **Returns**: An integer indicating success (1) or failure (0).

#### ADD_BUTTON

```rexx <!--addbutton.rexx-->
add_button(button_text,x-offset,y-offset)
```

- **Description**: Creates a new button with the specified text and adds it to the fixed container. The button is displayed at the specified coordinates. At present, the button size is fixed at 100x25 pixels, but this might be changed in the future.
- **Parameters**:
  - `button_text`: The text to display on the button (e.g., `"Click Me"`).
  - `x`: The X-coordinate for button placement (e.g., `10`).
  - `y`: The Y-coordinate for button placement (e.g., `10`).
- **Returns**: 
  - The index of the button in the global widgets array. 
  - Returns `-1` if the button cannot be created.
- **Example**:
```rexx <!--addbuttonexample.rexx-->
  button_index = add_button("Submit", 50, 100)
  if button_index < 0 then
      say "Error: Could not add button."
  ```

#### ADD_TEXT

```rexx <!--addtext.rexx-->
add_text(text,x-offset,y-offset)
```

- **Description**: Creates a new label and adds it to the fixed container. At present, the label size is fixed at 100x25 pixels, but this might be changed in the future.
- **Parameters**:
  - `text`: Text to display on the label.
  - `x`: X-coordinate for label placement.
  - `y`: Y-coordinate for label placement.
- **Returns**: The index of the label in the global widgets array.
- **Example**:
```rexx <!--addtextexample.rexx-->
  label_index = add_text("Hello World", 10, 50)
```

#### ADD_COMBO

```rexx <!--addcombo.rexx-->
add_combo(item-list,x-offset,y-offset)
```

- **Description**: Creates a new combo box and adds it to the fixed container. At present, the combo box size is fixed at 100x25 pixels, but this might be changed in the future.
- **Parameters**:
  - `item-list`: An array containing a list of items to add to the combo box (e.g., `["Option 1", "Option 2", "Option 3"]`).
  - `x`: X-coordinate for combo box placement.
  - `y`: Y-coordinate for combo box placement.
- **Returns**: The index of the combo box in the global widgets array.
- **Example**:
```rexx <!--addcomboexample-->
  combo_index = add_combo(["Option 1", "Option 2", "Option 3"], 10, 100)
```

#### ADD_LIST

```rexx <!--addlist.rexx-->
add_list(x-offset,y-offset,width,height)
```

- **Description**: Creates a new list box and adds it to the fixed container. At present, the list box size is fixed at 100x25 pixels, but this might be changed in the future.
- **Parameters**:
  - `x`: X-coordinate for list placement.
  - `y`: Y-coordinate for list placement.
  - `width`: Width of the list box.
  - `height`: Height of the list box.
- **Returns**: The index of the list in the global widgets array.
- **Example**:
```rexx <!--addlistexample-->
  list_index = add_list(10, 150, 200, 100)
```

#### ADD_EDIT

```rexx <!--addedit.rexx-->
add_edit(x-offset,y-offset,intitial-value)
```

- **Description**: Creates a new editable text field (entry) and adds it to the fixed container. At present, the entry size is fixed at 100x25 pixels, but this might be changed in the future.
- **Parameters**:
  - `x`: X-coordinate for entry placement.
  - `y`: Y-coordinate for entry placement.
  - `initial-value`: Initial text for the entry.
- **Returns**: The index of the entry in the global widgets array.
- **Example**:
  ```rexx <!--addeditexample.rexx-->
  edit_index = add_edit(10, 100, "Type here...")
  ```

#### LIST_ADD_ITEM

```rexx <!--addlistadd.rexx-->
list_add_item(list,x-offset,y-offset,bg_colour)
```

- **Description**: Adds a new item to the specified list.
- **Parameters**:
  - `list`: Index of the list to which the item will be added.
  - `text`: Text to display for the new item.
  - `bg_colour`: Background colour for the item (refer to the [X11 colours](#appendix-x11-colours) for available colours or use RGB in hexadecimal notation, e.g., `#RRGGBB`).
- **Returns**: An integer indicating success (1).
- **Example**:
```rexx <!--addlistaddexample.rexx-->
  call list_add_item(list_index, "Item 1", "lightblue")
```

#### LIST_GET_SELECTED

```rexx <!--addistgetselected.rexx-->
list_get_selected(list) 
```

- **Description**: Retrieves the index of the currently selected item in the specified list.
- **Parameters**:
  - `list`: Index of the list to check for selection.
- **Returns**: The index of the selected item or -1 if no selection is made.
- **Example**:
  ```rexx <!--addistgetselectedexample.rexx-->
  selected_index = list_get_selected(list_index)
  ```

#### LIST_GET_SELECTED_ITEM

```rexx <!--addlistgetselecteditem.rexx-->
list_get_selected_item(list)
```

- **Description**: Retrieves the text of the currently selected item in the specified list.
- **Parameters**:
  - `list`: Index of the list to check for selection.
- **Returns**: The text of the selected item or an empty string if no selection is made.
- **Example**:
```rexx <!--addlistgetselecteditemexample.rexx-->
  selected_text = list_get_selected_item(list_index)
```

#### LIST_SET_HEADER

```rexx <!--addlistsetheader.rexx-->
list_set_header(list,header_text,bg_colour)
```

- **Description**: Sets a header for the specified list.
- **Parameters**:
  - `list`: Index of the list to set the header for.
  - `header_text`: Text to display as the header.
  - `bg_colour`: Background colour of the header (refer to the [X11 colours](#appendix-x11-colours) for available colours or use RGB in hexadecimal notation).
- **Returns**: An integer indicating success (1).
- **Example**:

```rexx <!--addlistsetheader.rexx-->
  call list_set_header(list_index, "My List Header", "lightgray")
```

#### CLEANUP_GUI

```rexx <!--cleanupgui.rexx-->
cleanup_gui
```

- **Description**: Cleans up and destroys all widgets and the main window.
- **Returns**: None.
- **Example**:
```rexx <!--cleanupguiexample.rexx-->
  call cleanup_gui
```

#### GET_WIDGET_ADDRESS

```rexx <!--getwidgetaddress.rexx-->
get_widget_address(index)
```

- **Description**: Retrieves the address of a widget based on its index in the global widgets array.
- **Parameters**:
  - `index`: Index of the widget to retrieve.
- **Returns**: The address of the widget or 0 if the index is invalid.
- **Example**:
```rexx <!--getwidgetaddressexample.rexx-->
  widget_address = get_widget_address(button_index)
```

#### RGB_TO_HEX

```rexx <!--rgbtohex.rexx-->
rgb_to_hex(r, g, b)
```

- **Description**: Converts RGB values to a hexadecimal color string.
- **Parameters**:
  - `r`: Red component (0-255).
  - `g`: Green component (0-255).
  - `b`: Blue component (0-255).
- **Returns**: A string representing the color in hexadecimal format (e.g., `#RRGGBB`). Returns an empty string if the input values are out of range.
- **Example**:
```rexx <!--rgbtohexexample.rexx-->
  hex_color = rgb_to_hex(255, 0, 0)  // Returns "#FF0000"
```

#### SET_SENSITIVE

```rexx <!--setsensitive.rexx-->
set_sensitive(widget_index, sensitive)
```

- **Description**: Enables or disables a widget while keeping it visible.
- **Parameters**:
  - `widget_index`: Index of the widget to modify.
  - `sensitive`: 1 to enable the widget, 0 to disable it.
- **Returns**: 
  - `0` on success
  - `-1` if the widget index is invalid
- **Example**:

```rexx <!--setsensitiveexample.rexx-->
  /* Disable a widget */
  call set_sensitive button_index, 0
  
  /* Enable a widget */
  call set_sensitive button_index, 1
  ```

#### COPY_FILE

```rexx <!--guiex1-->
copy_file(source_path, destination_path)
```

- **Description**: Copies a file from the source path to the destination path.
- **Parameters**:
  - `source_path`: Path to the source file.
  - `destination_path`: Path where the file should be copied to.
- **Returns**: 
  - `0` on success
  - `-8` on failure
- **Example**:
```rexx <!--guiex2.rexx-->
  rc = copy_file("C:\source\file.txt", "C:\destination\file.txt")
  if rc = 0 then
      say "File copied successfully"
  else
      say "Error copying file"
  ```

### Dialog and Picker Services

#### FILE_PICK

```rexx <!--guiex3.rexx-->
file_pick(title, initial_dir, save_dialog, pattern)
```

- **Description**: Opens a file picker dialog to select or save a file.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `initial_dir`: Initial directory to open.
  - `save_dialog`: 0 for open dialog, 1 for save dialog.
  - `pattern`: File pattern to filter (e.g., "*.txt", "*.rexx").
- **Returns**: Selected file path as string, or empty string if cancelled.
- **Examples**:

```rexx <!--guiex4.rexx-->
  /* Basic file selection */
  filepath = file_pick("Select a File", "C:\Documents", 0, "*.rexx")
  
  /* Save dialog with specific extension */
  savepath = file_pick("Save As", "C:\Documents", 1, "*.txt")
  
  /* Multiple file patterns */
  files = file_pick("Select Document", "C:\Docs", 0, "*.doc;*.docx;*.txt")
  
  /* All files */
  anyfile = file_pick("Select Any File", "C:\Files", 0, "*.*")
```
- **Error Handling**:
```rexx <!--guiex5.rexx-->
  filepath = file_pick("Select File", initial_dir, 0, "*.rexx")
  if filepath = "" then do
      say "User cancelled file selection or an error occurred"
      return
  end
```
- **Tips**:
  - Always provide an initial directory for better user experience
  - Use clear, descriptive titles
  - Specify file patterns to help users find relevant files
  - Handle empty returns (user cancellation)

#### PATH_PICK

```rexx <!--guiex6.rexx-->
path_pick(title, initial_dir)
```

- **Description**: Opens a directory picker dialog to select a folder.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `initial_dir`: Initial directory to open.
- **Returns**: Selected directory path as string, or empty string if cancelled.
- **Examples**:
```rexx <!--guiex7.rexx-->
  /* Basic directory selection */
  dirpath = path_pick("Select Folder", "C:\Documents")
  
  /* Project directory selection with validation */
  projectDir = path_pick("Select Project Directory", "C:\Projects")
  if projectDir \= "" then do
      if directory_exists(projectDir) then
          say "Selected directory:" projectDir
      else
          say "Invalid directory selection"
  end
```
- **Error Handling**:
```rexx <!--guiex8.rexx-->
  dirpath = path_pick("Select Directory", initial_dir)
  if dirpath = "" then do
      say "User cancelled directory selection or an error occurred"
      return
  end
```
- **Tips**:
  - Use for selecting output directories
  - Validate directory existence after selection
  - Consider combining with FILE_PICK for complete file operations

#### DATE_PICK

```rexx <!--guiex9.rexx-->
date_pick(title, show_time, format)
```

- **Description**: Opens a calendar dialog to select a date and optionally time.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `show_time`: 1 to include time selection, 0 for date only.
  - `format`: Date/time format string.
- **Returns**: Selected date/time as formatted string, or empty string if cancelled.
- **Format Strings**:
  - `YYYY`: Four-digit year
  - `MM`: Two-digit month (01-12)
  - `DD`: Two-digit day (01-31)
  - `HH`: Hours (00-23)
  - `mm`: Minutes (00-59)
  - `ss`: Seconds (00-59)
- **Examples**:
```rexx <!--guiex10.rexx-->
  /* Date only selection */
  date = date_pick("Select Date", 0, "YYYY-MM-DD")
  
  /* Date and time selection */
  datetime = date_pick("Select Date and Time", 1, "YYYY-MM-DD HH:mm:ss")
  
  /* Custom format */
  custom = date_pick("Select Date", 0, "DD/MM/YYYY")
  
  /* With validation */
  selected_date = date_pick("Select Delivery Date", 1, "YYYY-MM-DD HH:mm")
  if selected_date \= "" then do
      if validate_future_date(selected_date) then
          say "Valid future date selected:" selected_date
      else
          say "Please select a future date"
  end
```
- **Error Handling**:
```rexx <!--guiex11.rexx-->
  date = date_pick("Select Date", 0, "YYYY-MM-DD")
  if date = "" then do
      say "User cancelled date selection or an error occurred"
      return
  end
```
- **Tips**:
  - Use clear format strings for your locale
  - Consider time zones when using time selection
  - Validate dates for specific requirements (future dates, working days, etc.)
  - Use consistent date formats throughout your application

#### LIST_PICK

```rexx <!--guiex12.rexx-->
list_pick(title, items, multi_select, message)
```

- **Description**: Opens a dialog with a list of items to select from.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `items`: Array of items to display in the list.
  - `multi_select`: 1 to allow multiple selections, 0 for single selection.
  - `message`: Optional message to display above the list.
- **Returns**: Selected item(s) as string (comma-separated if multiple), or empty string if cancelled.
- **Examples**:
```rexx <!--guiex13.rexx-->
  /* Single selection */
  options = ["Small", "Medium", "Large"]
  size = list_pick("Select Size", options, 0, "Choose your preferred size:")
  
  /* Multiple selection */
  toppings = ["Cheese", "Pepperoni", "Mushrooms", "Olives", "Onions"]
  selected = list_pick("Select Toppings", toppings, 1, "Choose your toppings:")
  
  /* With validation */
  if selected \= "" then do
      parse var selected first ',' rest
      say "Primary selection:" first
      if rest \= "" then
          say "Additional selections:" rest
  end
```
- **Error Handling**:
```rexx <!--guiex14.rexx-->
  /* Check for empty selection */
  result = list_pick("Select Items", items, 1, "Please select:")
  if result = "" then do
      say "No selection made"
      return
  end
  
  /* Handle multiple selections */
  if pos(',', result) > 0 then do
      say "Multiple items selected"
      do while result \= ""
          parse var result item ',' result
          say "Selected:" item
      end
  end
```
- **Tips**:
  - Keep lists reasonably sized (consider scrolling for long lists)
  - Provide clear instructions in the message parameter
  - Consider sorting items alphabetically
  - Use meaningful item names
  - Handle both single and multiple selection cases

#### COMBO_PICK

```rexx <!--guiex15.rexx-->
combo_pick(title, message, items)
```

- **Description**: Shows a dialog with a combo box for selection.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `message`: Message to display above the combo box.
  - `items`: Array of items to display in the combo box.
- **Returns**: Selected item as string, or empty string if cancelled.
- **Example**:
```rexx <!--guiex16.rexx-->
  items = ["Option 1", "Option 2", "Option 3"]
  selected = combo_pick("Choose Option", "Please select:", items)
  ```

#### TREE_PICK

```rexx <!--guiex17.rexx-->
tree_pick(title, message, items, parents, multi_select)
```

- **Description**: Shows a tree view dialog for hierarchical selection.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `message`: Message to display above the tree.
  - `items`: Array of items to display in the tree.
  - `parents`: Array indicating parent-child relationships.
  - `multi_select`: 1 to allow multiple selections, 0 for single selection.
- **Returns**: Selected item(s) as string (comma-separated if multiple), or empty string if cancelled.
- **Examples**:
```rexx <!--guiex18.rexx-->
  /* Basic tree structure */
  items.1 = "Root"
  items.2 = "Child1"
  items.3 = "Child2"
  items.4 = "Subchild1"
  parents.1 = ""
  parents.2 = "Root"
  parents.3 = "Root"
  parents.4 = "Child1"
  selected = tree_pick("Select Items", "Choose from tree:", items, parents, 0)
  
  /* File system example */
  items.1 = "Documents"
  items.2 = "Work"
  items.3 = "Personal"
  items.4 = "Reports"
  items.5 = "Letters"
  parents.1 = ""
  parents.2 = "Documents"
  parents.3 = "Documents"
  parents.4 = "Work"
  parents.5 = "Personal"
  files = tree_pick("File Browser", "Select files:", items, parents, 1)
  
  /* Organization chart */
  items.1 = "CEO"
  items.2 = "HR"
  items.3 = "IT"
  items.4 = "HR Manager"
  items.5 = "HR Staff"
  items.6 = "IT Manager"
  items.7 = "Developers"
  parents.1 = ""
  parents.2 = "CEO"
  parents.3 = "CEO"
  parents.4 = "HR"
  parents.5 = "HR"
  parents.6 = "IT"
  parents.7 = "IT"
  positions = tree_pick("Organization", "Select department:", items, parents, 0)
```
- **Error Handling**:
```rexx <!--guiex19.rexx-->
  if selected = "" then do
      say "No selection made or dialog cancelled"
      return
  end
  
  /* Handle multiple selections */
  do while selected \= ""
      parse var selected item ',' selected
      say "Processing:" item
  end
```
- **Tips**:
  - Keep tree depth reasonable (3-4 levels maximum for usability)
  - Use clear parent-child relationships
  - Consider using icons for different types of nodes
  - Provide clear hierarchy in item names
  - Handle both single and multiple selection modes appropriately

#### INPUT_PICK

```rexx <!--guiex20.rexx-->
input_pick(title, message, default_value, password_mode)
```

- **Description**: Shows an input dialog for text entry.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `message`: Message to display above the input field.
  - `default_value`: Default text in the input field.
  - `password_mode`: 1 to mask input (for passwords), 0 for normal text.
- **Returns**: Entered text as string, or empty string if cancelled.
- **Example**:
```rexx <!--guiex21.rexx-->
  /* Normal input */
  name = input_pick("Name Entry", "Enter your name:", "", 0)
  
  /* Password input */
  password = input_pick("Password", "Enter password:", "", 1)
```

#### FORM_PICK

```rexx <!--guiex21.rexx-->
form_pick(title, message, labels, defaults)
```

- **Description**: Shows a form dialog with multiple input fields.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `message`: Message to display above the form.
  - `labels`: Array of labels for each input field.
  - `defaults`: Array of default values for each input field.
- **Returns**: Comma-separated string of entered values, or empty string if cancelled.
- **Example**:
```rexx <!--guiex21.rexx-->
  labels = ["Name:", "Email:", "Phone:"]
  defaults = ["John Doe", "john@example.com", ""]
  result = form_pick("User Information", "Please enter your details:", labels, defaults)
```

#### PAGE_PICK

```rexx <!--guiex22.rexx-->
page_pick(title, pages, labels, defaults)
```

- **Description**: Shows a multi-page form dialog.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `pages`: Array of page titles.
  - `labels`: Array of labels for input fields on each page.
  - `defaults`: Array of default values for input fields.
- **Returns**: Comma-separated string of entered values, or empty string if cancelled.
- **Example**:
```rexx <!--guiex23.rexx-->
  pages = ["Personal", "Contact", "Preferences"]
  labels = ["Name,Age", "Email,Phone", "Theme,Language"]
  defaults = ["John Doe,30", "john@example.com,555-0123", "Dark,English"]
  result = page_pick("User Profile", pages, labels, defaults)
```

#### DIALOG_PICK

```rexx <!--guiex23.rexx-->
dialog_pick(title, message, buttons)
```

- **Description**: Shows a message dialog with custom buttons.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `message`: Message to display.
  - `buttons`: Array of button labels.
- **Returns**: Index of clicked button (1-based), or 0 if cancelled.
- **Example**:
```rexx <!--guiex24.rexx-->
  buttons = ["Yes", "No", "Cancel"]
  response = dialog_pick("Confirm", "Do you want to save?", buttons)
```

#### NOTIFY_PICK

```rexx <!--guiex25.rexx-->
notify_pick(title, message, type)
```

- **Description**: Shows a notification/message dialog.
- **Parameters**:
  - `title`: Title of the notification.
  - `message`: Message to display.
  - `type`: Type of notification ("info", "warning", "error", "question").
- **Returns**: Empty string.
- **Example**:
```rexx <!--guiex26.rexx-->
  call notify_pick "Success", "Operation completed successfully", "info"
  call notify_pick "Warning", "Low disk space", "warning"
```

#### SPLASH_PICK

```rexx <!--guiex27.rexx-->
splash_pick(title, message, duration, width, height, image_path)
```

- **Description**: Shows a splash screen with optional image.
- **Parameters**:
  - `title`: Title of the splash window.
  - `message`: Message to display.
  - `duration`: Display duration in seconds (0 for manual close).
  - `width`: Width of the splash window.
  - `height`: Height of the splash window.
  - `image_path`: Path to an image file to display (optional).
- **Returns**: Empty string.
- **Example**:
```rexx <!--guiex28.rexx-->
  /* Show splash screen for 3 seconds */
  call splash_pick "Welcome", "Loading...", 3, 400, 300, "logo.png"
```

#### TEXT_DISPLAY

```rexx <!--guiex29.rexx-->
text_display(title, message, item_text)
```

- **Description**: Shows a simple text display dialog.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `message`: Message to display above the text.
  - `item_text`: Text content to display (can include newlines).
- **Returns**: Empty string.
- **Example**:
```rexx <!--guiex30.rexx-->
  text = "First line\nSecond line\nThird line"
  call text_display "Document", "Content:", text
```

#### TEXT_DISPLAY_PICK

```rexx <!--guiex31.rexx-->
text_display_pick(title, message, item_texts)
```

- **Description**: Shows a dialog with formatted text display.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `message`: Message to display above the text.
  - `item_texts`: Array of text items to display.
- **Returns**: Empty string.
- **Example**:
```rexx <!--guiex32.rexx-->
  texts = ["First paragraph", "Second paragraph", "Third paragraph"]
  call text_display_pick "Document", "Preview:", texts
```

#### COLOR_PICK

```rexx <!--guiex33.rexx-->
color_pick(title, message, default_color)
```

- **Description**: Shows a color picker dialog.
- **Parameters**:
  - `title`: Title of the dialog window.
  - `message`: Message to display above the color picker.
  - `default_color`: Initial color (in hex format or X11 color name).
- **Returns**: Selected color in hex format (#RRGGBB), or empty string if cancelled.
- **Example**:
```rexx <!--guiex34.rexx-->
  color = color_pick("Choose Color", "Select background color:", "#FF0000")
```

#### TREE_DIAGRAM

```rexx <!--guiex35.rexx-->
tree_diagram(items, parents)
```

- **Description**: Shows a graphical tree diagram.
- **Parameters**:
  - `items`: Array of items to display in the diagram.
  - `parents`: Array indicating parent-child relationships.
- **Returns**: Empty string.
- **Example**:
```rexx <!--guiex36.rexx-->
  items = ["CEO", "Manager1", "Manager2", "Employee1", "Employee2"]
  parents = ["", "CEO", "CEO", "Manager1", "Manager2"]
  call tree_diagram items, parents
```

#### ADD_GRAPH

```rexx <!--guiex37.rexx-->
add_graph(x, y, width, height, function_type,x1,y1,x2,y2,x3,y3)
```

- **Description**: Creates a widget to display mathematical functions and graphs.
- **Parameters**:
  - `x`: X-coordinate for graph placement.
  - `y`: Y-coordinate for graph placement.
  - `width`: Width of the graph area.
  - `height`: Height of the graph area.
  - `x1:`  x-value-1.
  - `y1:`  y-value related to x-value-1.
  - `x2:`  x-value-2.
  - `y2:`  y-value related to x-value-2.
  - `x3:`  x-value-3.
  - `y3:`  y-value related to x-value-3.
  A maximum of three individual graph elements can be added to a single graph widget. By default, these graphs are hidden and must be triggered using the upd_graph function to be displayed.
    The passed x/y values must be float arrays. The sizing of the x/y axes is determined by the x1/y1 arrays. Therefore, it is essential that the ranges of x2/y2 and x3/y3 align properly.

- **Returns**: The index of the graph widget in the global widgets array.
- **Example**:
```rexx <!--guiex38.rexx-->
  graph1 = add_graph(10, 10, 300, 200,x1,y1,x2,y2,x3,y3)
```
- **Tips**:
  - Use appropriate size for visibility (recommended minimum 200x200)
  - Consider window size when placing graphs
  - Multiple graphs can be added to the same window

#### ADD_R2CHART
```rexx <!--guiex39.rexx-->
add_r2chart(x, y, width, height, function_type,x1,y1,x2,y2,x3,y3)
```

- **Description**: Creates a widget to display ℝ²⁺ Chart – Specifies only the first quadrant (non-negative real numbers). Can display up to three graphs in the same widget.
- **Parameters**:
  - `x`: X-coordinate for graph placement.
  - `y`: Y-coordinate for graph placement.
  - `width`: Width of the graph area.
  - `height`: Height of the graph area.
  - `x1`: Array of x coordinates for the first graph.
  - `y1`: Array of y coordinates for the first graph.
  - `x2`: Array of x coordinates for the second graph (optional).
  - `y2`: Array of y coordinates for the second graph (optional).
  - `x3`: Array of x coordinates for the third graph (optional).
  - `y3`: Array of y coordinates for the third graph (optional).
- **Returns**: The index of the graph widget in the global widgets array.
- **Example**:
```rexx <!--guiex40.rexx-->
  /* Create arrays for the graphs */
  j=1
  do i=-20 to 20
     x.j=i/10
     y.j=x.j*x.j    /* Quadratic function */
     j=j+1
  end

  /* Create graph widget with two functions */
  graph = add_r2chart(10, 30, 570, 550, x, y, a, b)
  
  /* Update graph colors */
  call update_graph graph, 1, 1, 3, "yellow"  /* First graph */
  call update_graph graph, 2, 1, 3, "blue"    /* Second graph */
```


### UPDATE_GRAPH
```rexx <!--guiex41.rexx-->
update_graph(graph_index, graph_num, line_type, line_width, color)
```

- **Description**: Updates the appearance of a specific graph within a graph widget. Since there may be more than one graph in the widget, this function only prepares the appearance of the specified graph. This is because the drawing function operates on the entire widget, which contains all the graphs. To complete the drawing process, you can use a value of -1 for the graphnum parameter.
- **Parameters**:
    - `graph_index`: Index of the graph widget.
    - `graph_num`: Which graph to update (1-3) or -1 to perform the draw process.
    - `line_type`: Type of line (1=solid, 2=dots).
    - `line_width`: Width of the line/dots in pixels.
    - `color`: Color name (X11 color) or hex value.
- **Returns**: None.
- **Example**:
```rexx <!--guiex42.rexx-->
  /* Update first graph to yellow solid line */
  call update_graph graph, 1, 1, 3, "yellow"
  
  /* Update second graph to blue dashed line */
  call update_graph graph, 2, 2, 1, "blue"
```

### Common Issues with Graphs

1. **Graph Not Updating**: Ensure that the correct graph index is being used in `update_graph`. Each graph should maintain its own state.
2. **Graphs Overlapping**: Make sure to set proper coordinates and sizes for each graph to avoid overlap.

### Debugging Tips

1. **Debug Output**: Use debug logging to trace function calls and variable states.
2. **Event Processing**: Ensure the event loop is running to allow for updates and redraws.
3. **Widget States**: Verify that widget states are being managed correctly, especially when updating or redrawing.


--- 

## Common Patterns and Best Practices

### GUI Layout Guidelines

### Error Handling Strategies

### Common Usage Patterns

### Performance Tips

---

## Real-World Examples

### Form Application

### File Processing Application

### Configuration Dialog

Here's an example of a settings/configuration dialog:

```rexx <!--guiex42.rexx-->
/* Initialize GUI */
call init_window "Application Settings", 500, 600

/* Create tabs or sections */
call add_text "General Settings", 10, 10

/* Theme Selection */
call add_text "Theme:", 10, 40
themes.1 = "Light"
themes.2 = "Dark"
themes.3 = "System Default"
theme_combo = add_combo(themes, 10, 60)

/* Language Selection */
call add_text "Language:", 10, 100
langs.1 = "English"
langs.2 = "Spanish"
langs.3 = "French"
lang_combo = add_combo(langs, 10, 120)

/* File Paths */
call add_text "Default Save Location:", 10, 160
path_field = add_edit(10, 180, 380)
path_btn = add_button("Browse", 400, 180)

/* Preferences */
call add_text "Preferences", 10, 220
auto_save = add_checkbox("Auto-save files", 10, 250)
show_toolbar = add_checkbox("Show toolbar", 10, 280)
enable_logging = add_checkbox("Enable logging", 10, 310)

/* Advanced Settings */
call add_text "Advanced", 10, 350
buffer_size = add_edit(10, 380, 100)
call add_text "Buffer Size (KB)", 120, 385

/* Action Buttons */
save_btn = add_button("Save Settings", 10, 500)
cancel_btn = add_button("Cancel", 120, 500)
default_btn = add_button("Restore Defaults", 230, 500)

/* Status Bar */
status_bar = add_status_bar()

/* Show window */
call show_window

/* Main event loop */
do forever
    event = process_events(500)
    if event < 0 then leave
    
    else if event = path_btn then do
        new_path = path_pick("Select Default Save Location", "C:\")
        if new_path \= "" then
            call set_text path_field, new_path
    end
    
    else if event = save_btn then do
        /* Validate settings */
        buffer = get_widget_address(buffer_size)
        if \datatype(buffer, 'W') | buffer < 1 then do
            call notify_pick "Error", "Invalid buffer size", "error"
            iterate
        end
        
        /* Save settings */
        if save_config() then do
            call set_status status_bar, "Settings saved successfully"
            call notify_pick "Success", "Settings have been saved", "info"
            leave
        end
    end
    
    else if event = default_btn then do
        if dialog_pick("Confirm", "Restore default settings?", ["Yes", "No"]) = 1 then
            call restore_defaults
    end
    
    else if event = cancel_btn then
        leave
end

/* Helper function to save configuration */
save_config: procedure
    /* Implementation of save logic */
return 1

/* Helper function to restore defaults */
restore_defaults: procedure
    call set_text buffer_size, "1024"
    call set_sensitive auto_save, 1
    call set_sensitive show_toolbar, 1
    call set_sensitive enable_logging, 0
return
```

## Troubleshooting

### Common Issues

1. **Window Not Showing**
   - Check if `show_window` was called
   - Verify window dimensions are reasonable
   - Ensure GTK is properly initialized

2. **Widget Creation Failures**
```rexx <!--guiex43.rexx-->
   button = add_button("Click Me", 10, 10)
   if button < 0 then do
       call notify_pick "Error", "Failed to create button", "error"
       return
   end
   ```

3. **Event Processing Issues**
   - Ensure event loop is properly implemented
   - Check event token values
   - Verify widget indices

4. **Memory Issues**
   - Call `cleanup_gui` when closing
   - Don't exceed maximum widget count (128)
   - Free resources properly

### Debugging Tips

1. **Event Debugging**
```rexx <!--guiex44.rexx-->
   /* Add debug output */
   do forever
       event = process_events(500)
       say "Event Token:" event
       if event < 0 then leave
   end
   ```

2. **Widget State Verification**
```rexx <!--guiex45.rexx-->
   /* Check widget status */
   widget_addr = get_widget_address(widget_index)
   if widget_addr = 0 then
       say "Widget not found or invalid"
```

3. **Status Updates**
```rexx <!--guiex46.rexx-->
   /* Add status messages */
   call set_status status_bar, "Processing event:" event
```

### Known Limitations

1. **Widget Limits**
   - Maximum 128 widgets per window
   - Fixed widget sizes (100x25 pixels for most widgets)
   - Limited layout options (fixed positioning only)

2. **Event Handling**
   - Single event loop per window
   - No asynchronous event processing
   - Limited event types

3. **Platform Specific**
   - Some features may work differently on different platforms
   - Font and color rendering may vary
   - File paths need platform-specific handling

---

## Appendix

### Function Index

### X11 Colours

This section provides a list of commonly used X11 colours that you can use in your GTK application. You can specify these colours by name or use their RGB values in hexadecimal notation (e.g., `#RRGGBB`).

### Basic Colours
- **`black`**: Represents the color black.
- **`white`**: Represents the color white.
- **`red`**: Represents the color red.
- **`green`**: Represents the color green.
- **`blue`**: Represents the color blue.
- **`yellow`**: Represents the color yellow.
- **`cyan`**: Represents the color cyan.
- **`magenta`**: Represents the color magenta.

### Shades of Gray
- **`gray`**: A medium shade of gray.
- **`lightgray`**: A lighter shade of gray.
- **`darkgray`**: A darker shade of gray.
- **`dimgray`**: A dimmer shade of gray.
- **`slategray`**: A bluish-gray color.
- **`light slate gray` **: A light bluish-gray color.
