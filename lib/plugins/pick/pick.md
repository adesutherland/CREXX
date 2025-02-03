The project is based on GTK 3.0, which theoretically runs on all operating systems. I have successfully installed it on Windows and adapted it in the CMake file. However, I am uncertain how to commit these changes to Git without interfering with the existing Git-based build process. Someone may have a suggestion.

# CREXX Pick Dialogs Documentation

## Table of Contents
- [File pick](#file-pick)
- [Path pick](#path-pick)
- [Date/Time pick](#datetime-pick)
- [List pick](#list-pick)
- [Dialog pick](#dialog-pick)
- [Input pick](#input-pick)
- [Form pick](#form-pick)
- [Notification pick](#notification-pick)
- [Combo pick](#combo-pick)
- [Page pick](#page-pick)
- [Tree pick](#tree-pick)
- [Text Display](#text-display)
- [Tree Diagram](#tree-diagram)

## File pick

Opens a dialog for selecting files or saving a file.

### Function
file_pick(title, initial_dir, save_dialog, filter_patterns, default_name, allow_multiple, select_folder)

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| initial_dir | string | Starting directory (empty for current) |
| save_dialog | int | 0=open file, 1=save file |
| filter_patterns | string | File patterns (e.g., "*.txt,*.doc") |
| default_name | string | Default filename for save dialog |
| allow_multiple | int | 0=single file, 1=multiple files |
| select_folder | int | 0=select files, 1=select folder |

### Returns
Selected file(s) or empty string if cancelled. Multiple files are separated by `|`

### Examples
```
/* Open a text file */
file = file_pick("Open Text File", "", 0, "*.txt", "", 0, 0)

/* Select multiple documents */
files = file_pick("Select Documents", "", 0, "*.doc,*.txt", "", 1, 0)
```

## Path pick

Opens a dialog for selecting directories only.

### Function
```c
path_pick(title, initial_dir)
```
### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| initial_dir | string | Starting directory (empty for current) |

### Returns
Selected directory path or empty string if cancelled

### Example
```c
path = path_pick("Select Output Directory", "")
```

## DateTime pick

Opens a calendar dialog with optional time selection.

### Function
date_pick(title, show_time, format)

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| show_time | int | 0=date only, 1=date and time |
| format | string | strftime format string (empty for default) |

### Default Formats
- Date only: `%Y-%m-%d`
- With time: `%Y-%m-%d %I:%M:%S %p`

### Features
- Week numbers display
- 12-hour format with AM/PM
- Seconds selection
- "Today" button

### Examples
```c
/* Date only */
date = date_pick("Select Date", 0, "")

/* Date and time */
datetime = date_pick("Select Date and Time", 1, "")

/* Custom format */
custom = date_pick("Select Date", 0, "%d/%m/%Y")
```

## List pick

Opens a dialog for selecting one or more items from a list.

### Function
list_pick(title, items, multi_select, message)

### Parameters
| Parameter | Type   | Description                              |
|-----------|--------|------------------------------------------|
| title | string | Dialog title                             |
| items | array  | array containing the list of items      |
| multi_select | int    | 0=single selection, 1=multiple selection |
| message | string | Optional message above list              |

### Returns
Selected item(s) or empty string if cancelled. Multiple selections are separated by `|`

### Examples
```c
/* Single selection */
fruits.1='Apple'
fruits.2='Banana'
fruits.3='Orange'

choice = list_pick("Select Fruit", fruits, 0, "Choose your favorite fruit:")

/* Multiple selection */
choices = list_pick("Select Fruits", fruits, 1, "Select all fruits you like:")
```

## Dialog pick

Opens a message dialog with custom buttons.

### Function
dialog_pick(title, message, buttons)

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| message | string | Message to display |
| buttons | string | Button labels separated by \| (e.g., "Yes\|No\|Cancel") |

### Returns
- "1" for first button
- "2" for second button
- "3" for third button, etc.
- Empty string if cancelled or closed

### Features
- Automatically wraps long messages
- Resizable window
- Modal behavior
- Default size of 300x150 pixels
- Default "OK" button if none specified

### Examples
```c
/* Simple OK dialog */
result = dialog_pick("Information", "Operation completed!", "OK")

/* Yes/No dialog */
result = dialog_pick("Question", "Save changes?", "Yes|No")

/* Custom buttons */
result = dialog_pick("Choose Action", "What would you like to do?", "Save|Don't Save|Cancel")
```

## Input pick

Opens a dialog with a text input field.

### Function
input_pick(title, message, default_value, password_mode)

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| message | string | Message above input field |
| default_value | string | Initial value for input field |
| password_mode | int | 0=normal text, 1=password field |

### Returns
Entered text or empty string if cancelled

### Features
- Optional password masking
- Pre-filled default value
- Text auto-selected when dialog opens
- Resizable window
- Modal behavior

### Examples
```c
/* Simple text input */
name = input_pick("Name Entry", "Please enter your name:", "", 0)

/* Password input */
pwd = input_pick("Password", "Enter your password:", "", 1)

/* With default value */
email = input_pick("Email", "Enter your email:", "user@example.com", 0)
```

## Form pick

Opens a dialog with multiple input fields.

### Function
form_pick(title, message, labels[], defaults[])

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| message | string | Optional message above form |
| labels | array | Labels for input fields |
| defaults | array | Default values for fields (optional) |

### Returns
Field values separated by `|` or empty string if cancelled

### Features
- Multiple input fields in a grid layout
- Optional default values
- First field auto-focused
- Resizable window
- Modal behavior

### Examples
```c
/* Simple form */
labels.1 = "Name"
labels.2 = "Email"
labels.3 = "Phone"

defaults.1 = ""
defaults.2 = "user@example.com"
defaults.3 = ""

result = form_pick("User Information", "Please fill in your details:", labels, defaults)
/* Returns: "John|john@email.com|555-1234" */
```

## Notification pick

Opens a styled notification dialog with an appropriate icon.

### Function
notify_pick(title, message, type)

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| message | string | Notification message |
| type | string | Notification type: "info", "warning", "error", or "success" |

### Features
- Color-coded messages
- Type-specific icons
- Automatic text wrapping
- Bold, larger text
- Modal behavior
- Consistent styling

### Returns
Always returns "1" (since it's just an OK dialog)

### Examples
```c
/* Information notification */
notify_pick("Info", "Operation completed successfully", "info")

/* Warning notification */
notify_pick("Warning", "Low disk space", "warning")

/* Error notification */
notify_pick("Error", "Could not connect to server", "error")

/* Success notification */
notify_pick("Success", "File uploaded successfully", "success")
```

## Combo pick

Opens a dialog with a dropdown selection box.

### Function
combo_pick(title, message, items[])

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| message | string | Message above combo box |
| items | array | List of items for dropdown |

### Returns
Selected item or empty string if cancelled

### Features
- Space-efficient dropdown interface
- Single selection only
- Quick keyboard navigation
- Compact display for long lists
- Modal behavior

### Examples
```c
/* Simple dropdown selection */
items.1 = "Small"
items.2 = "Medium"
items.3 = "Large"

size = combo_pick("Select Size", "Choose your preferred size:", items)
```

### Key Differences from List Pick

1. **Display Style**:
    - Combo: Shows one item at a time in a dropdown
    - List: Shows multiple items in a scrollable list

2. **Space Usage**:
    - Combo: More compact, good for limited space
    - List: Takes more space, shows more context

3. **Selection Type**:
    - Combo: Single selection only
    - List: Supports both single and multiple selection

4. **Use Cases**:
    - Combo: Best for simple choices from a known list
    - List: Better for multiple selections or when seeing all options at once is important

5. **Interaction**:
    - Combo: Quick keyboard typing to find items
    - List: Better for browsing and comparing options

## Page pick

Opens a dialog with multiple tabbed pages of input fields.

### Function
page_pick(title, pages[], labels[], defaults[])

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| pages | array | Field labels for each page, separated by \| |
| labels | array | Tab labels for each page |
| defaults | array | Default values for all fields (optional) |

### Returns
All field values separated by \| with pages separated by \|\|

### Features
- Multiple pages in tabs
- Form fields organized by page
- Optional default values
- Scrollable tabs for many pages
- Consistent field layout
- Modal behavior

### Examples
```c
/* Two-page form */
// Define pages
pages.1 = "Name|Email|Phone"           // Personal info page
pages.2 = "Street|City|State|Zip"      // Address page

// Define tab labels
labels.1 = "Personal Info"
labels.2 = "Address"

// Define defaults (optional)
defaults.1 = "John"
defaults.2 = "john@email.com"
defaults.3 = ""
defaults.4 = "123 Main St"
defaults.5 = "Anytown"
defaults.6 = "CA"
defaults.7 = "12345"

result = page_pick("User Registration", pages, labels, defaults)
/* Returns: "John|john@email.com|555-1234||123 Main St|Anytown|CA|12345" */
```

### Notes
- Fields within a page are separated by |
- Pages are separated by ||
- Each page can have different number of fields
- Tab scrolling appears automatically when needed

## Tree pick

Opens a dialog with a hierarchical tree view for selecting items.

### Function
tree_pick(title, message, items[], parents[], multi_select)

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| message | string | Message above tree |
| items | array | List of items to display |
| parents | array | Parent indices for each item (empty string for root items) |
| multi_select | int | 0=single selection, 1=multiple selection |

### Returns
Selected item indices separated by \| or empty string if cancelled

### Features
- Hierarchical tree display
- Optional multiple selection
- Expandable/collapsible nodes
- Scrollable view
- Modal behavior

### Examples
```rexx
/* Create a hierarchical category tree */
// Define items
items.1 = "Electronics"
items.2 = "Computers"
items.3 = "Laptops"
items.4 = "Desktops"
items.5 = "Phones"
items.6 = "Smartphones"
items.7 = "Basic Phones"

// Define parent relationships (empty=root, otherwise parent's index)
parents.1 = ""        /* Electronics (root) */
parents.2 = "1"       /* Computers under Electronics */
parents.3 = "2"       /* Laptops under Computers */
parents.4 = "2"       /* Desktops under Computers */
parents.5 = "1"       /* Phones under Electronics */
parents.6 = "5"       /* Smartphones under Phones */
parents.7 = "5"       /* Basic Phones under Phones */

/* This creates the following tree structure:
Electronics
  ├── Computers
  │     ├── Laptops
  │     └── Desktops
  └── Phones
        ├── Smartphones
        └── Basic Phones
*/

// Single selection
result = tree_pick("Select Category", "Choose a category:", items, parents, 0)
/* Returns index of selected item (e.g., "3" for Laptops) */

// Multiple selection
result = tree_pick("Select Categories", "Choose categories:", items, parents, 1)
/* Returns indices of selected items (e.g., "3|4" for Laptops and Desktops) */
```

### Notes
- Parent indices refer to array positions (1-based)
- Empty parent means root level item
- Tree automatically expands to show selections
- Returns original array indices for selected items
- Parent relationships must be valid (parent index must exist)
- Items are displayed in the order they appear in the array

## Text Display

Opens a dialog to display multiple text items in a scrollable list format.

### Function
text_display_pick(title, message,item_texts[])

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| title | string | Dialog title |
| message | string | Message above the list of items |
| item_texts | array | Array of text items to display |

### Returns
Always returns "1" when the dialog is closed.

### Features
- Scrollable list display of text items
- Customizable message at the top
- Modal behavior

### Examples
```c
/* Display a list of fruits */
item_texts.1 = "Apple"
item_texts.2 = "Banana"
item_texts.3 = "Cherry"
item_texts.4 = "Date"
item_texts.5 = "Elderberry"
item_texts.6 = "Fig"
item_texts.7 = "Grape"
item_texts.8 = "Honeydew"
item_texts.9 = "Kiwi"
item_texts.10 = "Lemon"
item_texts.11 = "Mango"
item_texts.12 = "Nectarine"
item_texts.13 = "Orange"
item_texts.14 = "Papaya"
item_texts.15 = "Quince"
item_texts.16 = "Raspberry"
item_texts.17 = "Strawberry"
item_texts.18 = "Tangerine"
item_texts.19 = "Ugli fruit"
item_texts.20 = "Vanilla bean"
item_texts.21 = "Watermelon"
item_texts.22 = "Xigua"
item_texts.23 = "Yellow squash"
item_texts.24 = "Zucchini"

rc=text_display("Fruit List", "Here are some fruits:", item_texts)
```

## Tree Diagram

Creates an ASCII visualization of a hierarchical tree structure.

### Function
tree_diagram(items[], parents[])

### Parameters
| Parameter | Type | Description |
|-----------|------|-------------|
| items | array | List of items to display |
| parents | array | Parent indices for each item (empty string for root items) |

### Returns
String containing the ASCII tree diagram

### Features
- Visual representation of parent-child relationships
- Uses box-drawing characters (├, │, └, ─) for tree lines
- Proper indentation of nested levels
- Clear visualization of hierarchical structures

### Examples
```rexx
/* Create a hierarchical category tree */
// Define items
items.1 = "Electronics"
items.2 = "Computers"
items.3 = "Laptops"
items.4 = "Desktops"
items.5 = "Phones"
items.6 = "Smartphones"
items.7 = "Basic Phones"

// Define parent relationships
parents.1 = ""        /* Electronics (root) */
parents.2 = "1"       /* Computers under Electronics */
parents.3 = "2"       /* Laptops under Computers */
parents.4 = "2"       /* Desktops under Computers */
parents.5 = "1"       /* Phones under Electronics */
parents.6 = "5"       /* Smartphones under Phones */
parents.7 = "5"       /* Basic Phones under Phones */

// Generate and display the tree diagram
diagram = tree_diagram(items, parents)
say diagram

/* Output:
Electronics
├── Computers
│   ├── Laptops
│   └── Desktops
└── Phones
    ├── Smartphones
    └── Basic Phones
*/
```

### Notes
- First item is always treated as the root
- Parent indices refer to array positions (1-based)
- Empty parent string indicates root level item
- Parent relationships must be valid (parent index must exist)
- Items are displayed in the order they appear in the array

---
*Note: All dialogs are modal and will wait for user input before returning.*

## Requirements

- GTK 3.x development libraries
- Installation:
    - Ubuntu/Debian: `sudo apt-get install libgtk-3-dev`
    - Fedora: `sudo dnf install gtk3-devel`
    - macOS: `brew install gtk+3`
    - Windows: Install GTK through MSYS2

