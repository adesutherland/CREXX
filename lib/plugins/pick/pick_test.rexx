/* PICK Plugin Test */
options levelb
import pick
import rxfnsb


/* Define items */
items.1 = "Electronics"
items.2 = "Computers"
items.3 = "Laptops"
items.4 = "Desktops"
items.5 = "Phones"
items.6 = "Smartphones"
items.7 = "Basic Phones"
items.8 = "PC Phones"

/* Define parent relationships */
parents.1 = ""        /* Electronics (root) */
parents.2 = "1"       /* Computers under Electronics */
parents.3 = "2"       /* Laptops under Computers */
parents.4 = "2"       /* Desktops under Computers */
parents.5 = "1"       /* Phones under Electronics */
parents.6 = "5"       /* Smartphones under Phones */
parents.7 = "5"       /* Basic Phones under Phones */
parents.8 = "5"       /* Basic Phones under Phones */

tree = tree_diagram(items, parents)
say tree
rc=text_display("Tree","Tree Structure",tree)
exit

rc=notify_pick("This is a Notify Pick", "Let's see customised dialogs", "info")
name = file_pick('Open File', 'C:\TEMP\CREXX\', 0)
rc=notify_pick("This is a Notification", "We have selected file: "name, "info")
name = path_pick('Select Path', 'C:\TEMP\CREXX')
rc=notify_pick("Path chosen", "We have selected path: "name, "info")
rc=notify_pick("Trigger", "Now let's trigger SUCCESS/WARNING/ERROR messages", "info")
/* Warning notification */
say notify_pick("Warning", "Low disk space", "warning")
/* Error notification */
say notify_pick("Error", "Could not connect to server", "error")
/* Success notification */
say notify_pick("Success", "File uploaded successfully", "success")

say notify_pick("Page update", "Let's update Variables via a page", "info")
pages.1 = "Name|Email|Phone"           ##Personal info page
pages.2 = "Street|City|State|Zip"      ## Address page

## Define tab labels
labels.1 = "Personal Info"
labels.2 = "Address"

## Define defaults (optional)
defaults.1 = "John"
defaults.2 = "john@email.com"
defaults.3 = ""
defaults.4 = "123 Main Street"
defaults.5 = "Anytown"
defaults.6 = "CA"
defaults.7 = "12345"
say page_pick("User Registration", pages, labels, defaults)

items.1 = "Small"
items.2 = "Medium"
items.3 = "Large"
size = combo_pick("Select Size", "Choose your preferred size:", items)
say notify_pick("This was combo pick", "We have chosen "size, "info")


 say notify_pick("update via Form", "Let's update Variables via a Form", "info")

labels.1 = "Name"
labels.2 = "Email"
labels.3 = "Phone"

defaults.1 = ""
defaults.2 = "user@example.com"
defaults.3 = ""

result = form_pick("User Information", "Please fill in your details:", labels, defaults)
labels.1 = "Name"
labels.2 = "Email"
labels.3 = "Phone"

defaults.1 = ""
defaults.2 = "user@example.com"
defaults.3 = ""
result = form_pick("User Information", "Please fill in your details:", labels, defaults)

name = input_pick("Name Entry", "Please enter your name:", "", 0)
say notify_pick("This is a Notify Pick", "We requested a single input line: "name, "info")

/* Password input */
pwd = input_pick("Password", "Enter your password:", "", 1)
say notify_pick("This is a Notify Pick", "We entered a password "pwd, "info")

/*
/* With default value */
email = input_pick("Email", "Enter your email:", "user@example.com", 0)
say email
*/



/* Show a dialog with custom buttons */
result = dialog_pick("This is a multiple choice dialog", "Do you want to proceed?", "Yes|No|Cancel|Maybe")
say notify_pick("This is a Notify Pick", "This was a simple Dialog to request action, we have selected: "result". item", "info")

say notify_pick("This is a Notify Pick", "And there is much more ...", "info")
/* Returns: "1" for first button, "2" for second button, etc., or "" if closed */


do i=1 to 25
   list.i='List item 'i
end
choice = list_pick("Select Item",list, 0, "Please select your favorite item")
say notify_pick("This is a Notify Pick", "we selected item: "choice, "info")

/* Multiple selection */
choices = list_pick("Select multiple items", list, 1, "Select all items you like")
say notify_pick("This is a Notify Pick", "we selected multiple items: "choices, "info")
SAY 'Selected items: 'choices

/* Date only picker */
date = date_pick("Select Date", 0, "")  /* Default format: YYYY-MM-DD */
rc= notify_pick("This is a Notify Pick", "we selected date: "date, "info")

/* Date and time picker */
datetime = date_pick("Select Date and Time", 1, "")  /* Default: YYYY-MM-DD HH:MM */
rc= notify_pick("This is a Notify Pick", "we selected date time: "datetime, "info")

/* Custom format */
custom = date_pick("Select Date", 0, "%d/%m/%Y")  /* Format: DD/MM/YYYY */
rc= notify_pick("This is a Notify Pick", "we selected custom date time: "custom, "info")


/* Define items */
items.1 = "Electronics"
items.2 = "Computers"
items.3 = "Laptops"
items.4 = "Desktops"
items.5 = "Phones"
items.6 = "Smartphones"
items.7 = "Basic Phones"
items.8 = "PC Phone"

/* Define parent relationships */
parents.1 = ""        /* Electronics (root) */
parents.2 = "1"       /* Computers under Electronics */
parents.3 = "2"       /* Laptops under Computers */
parents.4 = "2"       /* Desktops under Computers */
parents.5 = "1"       /* Phones under Electronics */
parents.6 = "5"       /* Smartphones under Phones */
parents.7 = "5"       /* Basic Phones under Phones */
parents.8 = "5"       /* Basic Phones under Phones */

diagram.1 = tree_diagram(items, parents)
say diagram.1
rc=text_display_pick("Tree","Tree Structure",diagram)

result = tree_pick("Select Category", "Choose a category:", items, parents, 0)
rc= notify_pick("We picked from Tree", "we selected: "result, "info")
/* Returns index of selected item (e.g., "3" for Laptops) */
/*
/* Multiple selection */
result = tree_pick("Select Categories", "Choose categories:", items, parents, 1)
*/
exit