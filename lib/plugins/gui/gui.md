# CREXX GUI Prerequisites 

## Table of Contents
1. [Overview](#overview)
2. [Installation Instructions](#installation-instructions)
   - [For Linux](#for-linux)
   - [For Windows](#for-windows)
3. [CREXX GUI Documentation](#crexx-gui-documentation)
   - [Overview](#overview-1)
   - [Available CREXX Functions](#available-crexx-functions)
     - [INIT_WINDOW](#init_window)
     - [ADD_BUTTON](#add_button)
     - [ADD_TEXT](#add_text)
     - [ADD_COMBO](#add_combo)
     - [ADD_LIST](#add_list)
     - [ADD_EDIT](#add_edit)
     - [LIST_ADD_ITEM](#list_add_item)
     - [LIST_GET_SELECTED](#list_get_selected)
     - [LIST_GET_SELECTED_ITEM](#list_get_selected_item)
     - [LIST_SET_HEADER](#list_set_header)
     - [CLEANUP_GUI](#cleanup_gui)
     - [GET_WIDGET_ADDRESS](#get_widget_address)
     - [RGB_TO_HEX](#rgb_to_hex)
4. [Error Handling](#error-handling)
5. [CREXX Example Documentation](#crexx-example-documentation)
   - [Overview](#overview-2)
   - [Example Usage](#example-usage)
6. [Notes](#notes)
7. [Appendix: X11 Colours](#appendix-x11-colours)

## Overview

This documentation provides an overview of a lightweight graphical user interface (GUI) for CREXX, focusing on the most essential widgets. 
It contains two files: `gui.c`, which implements the GUI functionality using GTK, and `gui_test.rexx`, which demonstrates how to use the procedures defined in `gui.c`.

---

## Installation Instructions

To set up the environment for running the GUI application, follow these steps:

### For Linux
1. **Install GTK**: Ensure your system has GTK installed. You can install it using your package manager. For example:
   - On Ubuntu/Debian:
     ```bash
     sudo apt-get install libgtk-3-dev
     ```
   - On Fedora:
     ```bash
     sudo dnf install gtk3-devel
     ```
   - On macOS using Homebrew:
     ```bash
     brew install gtk+3
     ```

2. **Compile the Code**: Use a C compiler to compile `gui.c`. For example:
   ```bash
   gcc -o gui gui.c `pkg-config --cflags --libs gtk+-3.0`
   ```

### For Windows

1. **Install GTK**: Download the GTK installer for Windows from the [GTK website](https://www.gtk.org/download/windows.php). Follow the instructions to install GTK on your system.

2. **Set Environment Variables**: After installation, you may need to set the `PATH` environment variable to include the GTK `bin` directory. This allows you to run GTK applications from the command line.

3. **Compile the Code**: Use a C compiler like MinGW or MSYS2 to compile `gui.c`. Open a terminal and run:
   ```bash
   gcc -o gui gui.c `pkg-config --cflags --libs gtk+-3.0`
   ```

---

## CREXX GUI Documentation

### Overview

`gui.c` is a source file for a GTK-based GUI application designed for the crexx/pa plugin architecture. It provides various procedures to create and manage GUI components such as buttons, labels, combo boxes, lists, and editable text fields. The file also includes functionality to handle user events and manage widget addresses. At present, the size of most of the widgets is fixed at 100x25 pixels, but this might be changed in the future.

### Available CREXX Functions 

#### INIT_WINDOW

```
init_window(title,width,height)(title,width,height)
```

- **Description**: Initializes the main application window and sets up the GTK environment.
- **Parameters**:
  - `title`: Title of the window.
  - `width`: Width of the window.
  - `height`: Height of the window.
- **Returns**: An integer indicating success (1) or failure (0).

#### ADD_BUTTON

```
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
  ```rexx
  button_index = add_button("Submit", 50, 100)
  if button_index < 0 then
      say "Error: Could not add button."
  ```

#### ADD_TEXT

```
add_text(text,x-offset,y-offset)
```

- **Description**: Creates a new label and adds it to the fixed container. At present, the label size is fixed at 100x25 pixels, but this might be changed in the future.
- **Parameters**:
  - `text`: Text to display on the label.
  - `x`: X-coordinate for label placement.
  - `y`: Y-coordinate for label placement.
- **Returns**: The index of the label in the global widgets array.
- **Example**:
  ```rexx
  label_index = add_text("Hello World", 10, 50)
  ```

#### ADD_COMBO

```
add_combo(item-list,x-offset,y-offset)
```

- **Description**: Creates a new combo box and adds it to the fixed container. At present, the combo box size is fixed at 100x25 pixels, but this might be changed in the future.
- **Parameters**:
  - `item-list`: An array containing a list of items to add to the combo box (e.g., `["Option 1", "Option 2", "Option 3"]`).
  - `x`: X-coordinate for combo box placement.
  - `y`: Y-coordinate for combo box placement.
- **Returns**: The index of the combo box in the global widgets array.
- **Example**:
  ```rexx
  combo_index = add_combo(["Option 1", "Option 2", "Option 3"], 10, 100)
  ```

#### ADD_LIST

```
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
  ```rexx
  list_index = add_list(10, 150, 200, 100)
  ```

#### ADD_EDIT

```
add_edit(x-offset,y-offset,intitial-value)
```

- **Description**: Creates a new editable text field (entry) and adds it to the fixed container. At present, the entry size is fixed at 100x25 pixels, but this might be changed in the future.
- **Parameters**:
  - `x`: X-coordinate for entry placement.
  - `y`: Y-coordinate for entry placement.
  - `initial-value`: Initial text for the entry.
- **Returns**: The index of the entry in the global widgets array.
- **Example**:
  ```rexx
  edit_index = add_edit(10, 100, "Type here...")
  ```

#### LIST_ADD_ITEM

```
list_add_item(list,x-offset,y-offset,bg_colour)
```

- **Description**: Adds a new item to the specified list.
- **Parameters**:
  - `list`: Index of the list to which the item will be added.
  - `text`: Text to display for the new item.
  - `bg_colour`: Background colour for the item (refer to the [X11 colours](#appendix-x11-colours) for available colours or use RGB in hexadecimal notation, e.g., `#RRGGBB`).
- **Returns**: An integer indicating success (1).
- **Example**:
  ```rexx
  call list_add_item(list_index, "Item 1", "lightblue")
  ```

#### LIST_GET_SELECTED

```
list_get_selected(list) 
```

- **Description**: Retrieves the index of the currently selected item in the specified list.
- **Parameters**:
  - `list`: Index of the list to check for selection.
- **Returns**: The index of the selected item or -1 if no selection is made.
- **Example**:
  ```rexx
  selected_index = list_get_selected(list_index)
  ```

#### LIST_GET_SELECTED_ITEM

```
list_get_selected_item(list)
```

- **Description**: Retrieves the text of the currently selected item in the specified list.
- **Parameters**:
  - `list`: Index of the list to check for selection.
- **Returns**: The text of the selected item or an empty string if no selection is made.
- **Example**:
  ```rexx
  selected_text = list_get_selected_item(list_index)
  ```

#### LIST_SET_HEADER

```
list_set_header(list,header_text,bg_colour)
```

- **Description**: Sets a header for the specified list.
- **Parameters**:
  - `list`: Index of the list to set the header for.
  - `header_text`: Text to display as the header.
  - `bg_colour`: Background colour of the header (refer to the [X11 colours](#appendix-x11-colours) for available colours or use RGB in hexadecimal notation).
- **Returns**: An integer indicating success (1).
- **Example**:
  ```rexx
  call list_set_header(list_index, "My List Header", "lightgray")
  ```

#### CLEANUP_GUI

```
cleanup_gui
```

- **Description**: Cleans up and destroys all widgets and the main window.
- **Returns**: None.
- **Example**:
  ```rexx
  call cleanup_gui
  ```

#### GET_WIDGET_ADDRESS

```
get_widget_address(index)
```

- **Description**: Retrieves the address of a widget based on its index in the global widgets array.
- **Parameters**:
  - `index`: Index of the widget to retrieve.
- **Returns**: The address of the widget or 0 if the index is invalid.
- **Example**:
  ```rexx
  widget_address = get_widget_address(button_index)
  ```

#### RGB_TO_HEX

```
rgb_to_hex(r, g, b)
```

- **Description**: Converts RGB values to a hexadecimal color string.
- **Parameters**:
  - `r`: Red component (0-255).
  - `g`: Green component (0-255).
  - `b`: Blue component (0-255).
- **Returns**: A string representing the color in hexadecimal format (e.g., `#RRGGBB`). Returns an empty string if the input values are out of range.
- **Example**:
  ```rexx
  hex_color = rgb_to_hex(255, 0, 0)  // Returns "#FF0000"
  ```

--- 

## Error Handling

When using the functions in this GUI, it's important to handle potential errors gracefully. Here are some common error scenarios and how to handle them:

1. **Initialization Errors**: 
   - If `init_window` fails, it returns `0`. Always check the return value before proceeding.
   ```rexx
   if init_window("My App", 400, 300) = 0 then
       say "Error: Could not initialize the window."
   ```

2. **Widget Creation Errors**:
   - Functions like `add_button`, `add_text`, and others return `-1` if they fail to create a widget. Check the return value to ensure the widget was created successfully.
   ```rexx
   button_index = add_button("Click Me", 10, 10)
   if button_index < 0 then
       say "Error: Could not add button."
   ```

3. **List Operations**:
   - When adding items to a list or retrieving selected items, ensure that the list index is valid. If the index is out of bounds, handle it appropriately.
   ```rexx
   if list_index < 0 or list_index >= MAX_WIDGETS then
       say "Error: Invalid list index."
   ```

4. **General Error Handling**:
   - Implement a general error handling mechanism in your REXX scripts to catch and respond to unexpected issues.

---

## CREXX Example Documentation

### Overview

`gui_test.rexx` is a REXX script that demonstrates how to use the procedures defined in `gui.c`. It initializes the GUI, adds various widgets, and interacts with them.

### Example Usage

```
/* Initialize the GUI */
init_window("My Application", 400, 300)

/* Add widgets */
button_index = add_button("Click Me", 10, 10)  /* Get index of the button */
say "Button Index:" button_index

label_index = add_text("Hello World", 10, 50)  /* Get index of the label */
say "Label Index:" label_index

edit_index = add_edit(10, 100, "Type here...")  /* Get index of the edit field */
say "Edit Index:" edit_index

/* Add a list and items */
list_index = add_list(10, 150, 200, 100)  /* Get index of the list */
call list_add_item(list_index, "Item 1", "lightblue")  /* Use X11 colour */
call list_add_item(list_index, "Item 2", "lightgreen")  /* Use X11 colour */

/* Get the selected item from the list */
selected_text = list_get_selected_item(list_index)  /* Get the text of the selected item */
say "Selected Item:" selected_text

/* Convert RGB to Hex */
hex_color = rgb_to_hex(255, 0, 0)  /* Should return "#FF0000" */
say "Hex Color:" hex_color
```

### Notes

- Ensure that the maximum number of widgets does not exceed `128` to avoid overflow.
- The procedures are designed to be called from REXX scripts, allowing for dynamic GUI creation and management.
- Both X11 colour names and RGB colours in hexadecimal notation (e.g., `#RRGGBB`) are supported for colour parameters. For example:
  - `#FF0000` for red
  - `#00FF00` for green
  - `#0000FF` for blue
  - `#FFFF00` for yellow
  - `#FFFFFF` for white
  - `#000000` for black
- You can also use the `RGB()` function to specify colours, such as:
  - `RGB(255, 0, 0)` for red
  - `RGB(0, 255, 0)` for green
  - `RGB(0, 0, 255)` for blue

--- 

## Appendix: X11 Colours

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
- **`light slate gray`**: A lighter version of slate gray.

### Reds
- **`lightcoral`**: A light shade of coral red.
- **`salmon`**: A pinkish-red color.
- **`darksalmon`**: A darker shade of salmon.
- **`crimson`**: A strong red color.
- **`firebrick`**: A dark red color.
- **`indianred`**: A medium red color with a hint of brown.

### Greens
- **`lightgreen`**: A light shade of green.
- **`mediumseagreen`**: A medium shade of sea green.
- **`seagreen`**: A bluish-green color.
- **`forestgreen`**: A dark green color.
- **`darkgreen`**: A very dark shade of green.
- **`limegreen`**: A bright green color.

### Blues
- **`lightblue`**: A light shade of blue.
- **`skyblue`**: A light blue color resembling the sky.
- **`deepskyblue`**: A bright blue color.
- **`dodgerblue`**: A vibrant blue color.
- **`steelblue`**: A bluish-gray color.
- **`royalblue`**: A deep blue color.
- **`navy`**: A very dark shade of blue.

### Yellows
- **`lightyellow`**: A pale shade of yellow.
- **`khaki`**: A light brownish-yellow color.
- **`gold`**: A bright yellow color resembling gold.
- **`goldenrod`**: A darker shade of yellow.

### Purples
- **`plum`**: A medium purple color.
- **`violet`**: A light purple color.
- **`orchid`**: A light purple color with pinkish tones.
- **`purple`**: A standard purple color.
- **`mediumpurple`**: A medium shade of purple.
- **`darkviolet`**: A darker shade of violet.

### Others
- **`orange`**: A bright orange color.
- **`tan`**: A light brown color.
- **`peachpuff`**: A light peach color.
- **`lightpink`**: A light shade of pink.
- **`hotpink`**: A bright pink color.
- **`palevioletred`**: A pale shade of violet-red.

### Usage
You can use these colour names in your GTK application to set widget colours. For example:
```c
set_widget_background(my_button, "lightblue");
```
Or using RGB in hexadecimal notation:
```c
set_widget_background(my_button, "#ADD8E6"); // Light blue
```