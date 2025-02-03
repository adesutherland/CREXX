//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <gtk/gtk.h>
#include <glib.h>     // Add this for g_main_context functions

// Global variables to store widgets we need to access across functions
static GtkWidget *main_window = NULL;
static GtkWidget *main_fixed = NULL;
static GtkWidget *main_label = NULL;
static int last_event_token = 0;  // Track which button was clicked
static int next_event_token = 1;  // Counter for generating unique tokens
#define WINDOW_CLOSE_TOKEN -1  // Special token for window close event
static gboolean user_event_occurred = FALSE;

/*
#define MAX_BUTTONS 50  // Maximum number of buttons we'll support
static GtkWidget *buttons[MAX_BUTTONS] = {NULL};  // Array to store button widgets
static int button_count = 0;  // Keep track of how many buttons we've added
#define MAX_LABELS 50
static GtkWidget *labels[MAX_LABELS] = {NULL};
static int label_count = 0;

#define MAX_COMBOS 20
static GtkWidget *combos[MAX_COMBOS];
static int combo_count = 0;
static int last_combo_token = 0;  // Track which combo was changed
#define COMBO_CHANGED_TOKEN -2    // Special token for combo change events
*/
static GtkSettings *gtk_settings = NULL;
static GdkDisplay *gdk_display = NULL;
#define LIST_SELECTED_TOKEN -3  // Special token for list selection events
#define MAX_WIDGETS 128  // Define a maximum number of widgets
static GtkWidget *widgets[MAX_WIDGETS] = {NULL};  // Array to store widget addresses
static int widget_count = 0;  // Keep track of how many widgets have been added

static gboolean button_clicked(GtkWidget *widget, gpointer data) {
    GtkLabel *label = GTK_LABEL(data);
    static int count = 0;
    char text[100];
    
    count++;
    snprintf(text, sizeof(text), "Button clicked %d times!", count);
    gtk_label_set_text(label, text);
    
    // Get the button's token from its data
    last_event_token = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "button-token"));
    printf("click %d\n", last_event_token);
    user_event_occurred = TRUE;
    return TRUE;
}

static gboolean timeout_callback(gpointer data) {
    return FALSE;  // Return FALSE to remove the source
}

static gboolean window_close_callback(GtkWidget *widget, GdkEvent *event, gpointer data) {
    user_event_occurred = TRUE;
    last_event_token = WINDOW_CLOSE_TOKEN;
    return FALSE;  // Allow the window to be destroyed
}

static void combo_changed_callback(GtkComboBox *widget, gpointer data) {
    // Get the combo's token from its data
    int token= GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "combo-token"));
    user_event_occurred = TRUE;
    last_event_token = token;  // Use button_token mechanism for events
}

static void list_row_selected(GtkListBox *box, GtkListBoxRow *row, gpointer data) {
    if (!row) return;  // No selection
    
    int token = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(box), "list-token"));
    user_event_occurred = TRUE;
    last_event_token = token;
}

static void set_widget_background(GtkWidget *widget, const char *color_name) {
    GdkRGBA color;
    
    if (gdk_rgba_parse(&color, color_name)) {
        gtk_widget_override_background_color(widget, GTK_STATE_FLAG_NORMAL, &color);
    }
}

// Function to convert RGB values to a hexadecimal color string
PROCEDURE(rgb_to_hex) {
    int r=GETINT(ARG0);
    int g=GETINT(ARG1);
    int b=GETINT(ARG2);
    char hex_color[1+6+1];
  // Ensure RGB values are within the valid range
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
            RETURNSTRX(""); // Return NULL for invalid input
        }
  // Format the RGB values into a hexadecimal string
        snprintf(hex_color, sizeof(hex_color), "#%02X%02X%02X", r, g, b);
    RETURNSTRX(hex_color); // Return the hexadecimal color string
ENDPROC
}

PROCEDURE(init_window)
{
    // Reset counters when initializing window
    widget_count = 0;

    // Initialize GTK with proper error checking
    if (!gtk_init_check(NULL, NULL)) {
        RETURNINT(0);  // GTK initialization failed
    }
    
    // Store display reference
    gdk_display = gdk_display_get_default();
    if (!gdk_display) {
        RETURNINT(0);
    }
    
    // Store settings reference
    gtk_settings = gtk_settings_get_default();
    if (!gtk_settings) {
        RETURNINT(0);
    }
    
    // Hold references to prevent cleanup
    g_object_ref(gdk_display);
    g_object_ref(gtk_settings);
    
    // Create main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), GETSTRING(ARG0));
    gtk_window_set_default_size(GTK_WINDOW(main_window), GETINT(ARG1), GETINT(ARG2));
    
    // Connect delete-event signal
    g_signal_connect(main_window, "delete-event", G_CALLBACK(window_close_callback), NULL);
    
    // Create a fixed container and show it
    main_fixed = gtk_fixed_new();
    gtk_container_add(GTK_CONTAINER(main_window), main_fixed);
    gtk_widget_show(main_fixed);
    
    // Show the window itself
    gtk_widget_show(main_window);
    
    // Process initial events to ensure window is fully set up
    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }
    
    RETURNINT(1);  // Success
ENDPROC
}

PROCEDURE(add_button)
{
    if (widget_count >= MAX_WIDGETS) {
          // Handle error - too many buttons
        RETURNINTX(-1);
    }
    
    const char *button_text = GETSTRING(ARG0);
    int x = GETINT(ARG1);
    int y = GETINT(ARG2);
    
    // Create the button and show it immediately
    widgets[widget_count] = gtk_button_new_with_label(button_text);
    gtk_fixed_put(GTK_FIXED(main_fixed), widgets[widget_count], x, y);
    
    // Store the token with the button
    g_object_set_data(G_OBJECT(widgets[widget_count]),
                     "button-token", 
                     GINT_TO_POINTER(next_event_token));
    
    g_signal_connect(widgets[widget_count], "clicked", G_CALLBACK(button_clicked), main_label);
    gtk_widget_show(widgets[widget_count]);
    
    // Store the widget address
    if (widget_count < MAX_WIDGETS) {
        widgets[widget_count++] = widgets[widget_count];
    }
    
    RETURNINT(widget_count);  // Return the index+1 (to have number >0)  of the widget in the widgets array
ENDPROC
}

PROCEDURE(add_text) {
    if (widget_count >= MAX_WIDGETS) {
        // Handle error - too many buttons
        RETURNINTX(-1);
    }
    const char *label_text = GETSTRING(ARG0);
    int x = GETINT(ARG1);
    int y = GETINT(ARG2);
    
    // Create the label and show it immediately
    widgets[widget_count] = gtk_label_new(label_text);
    gtk_fixed_put(GTK_FIXED(main_fixed), widgets[widget_count], x, y);
    gtk_widget_show(widgets[widget_count]);  // Show the label immediately
    widget_count++;

    RETURNINT(widget_count);  // Return the index+1 (to have number >0)  of the widget in the widgets array
ENDPROC
}

PROCEDURE(show_window) {
    if (main_window) {
        // Show the fixed container first
        gtk_widget_show(main_fixed);
        
        // Show all stored widgets
        for (int i = 0; i < widget_count; i++) {
            if (widgets[i]) {
                gtk_widget_show(widgets[i]);
            }
        }
    // Show the main window and all its children
        gtk_widget_show(main_window);
        
        // Make sure everything is visible
        gtk_widget_show_all(main_window);
    }
ENDPROC
}

PROCEDURE(process_events) {
    // Get timeout value from argument, default to 250ms if not specified
    int timeout = GETINT(ARG0) * 1000; // Convert milliseconds to microseconds
    int gtk_timeout = 10 * 1000;      // 10ms sleep interval in microseconds
    int waited = 0;                   // Track elapsed time
    if (timeout <= 0) timeout = 250000; // Default timeout is 250ms

    // Reset the event flag and token
    user_event_occurred = FALSE;
    last_event_token = 0;

    // Process events and wait for user interaction or timeout
    while (waited < timeout) {
        // Process any pending events
        while (gtk_events_pending()) {
            gtk_main_iteration_do(FALSE);
            if (user_event_occurred) {
                RETURNINTX(last_event_token); // Return the token of the clicked button
            }
        }

        // If no events are processed, wait for a short period
        g_usleep(gtk_timeout);
        waited += gtk_timeout;
    }

    RETURNINT(0); // Timeout occurred (using 0 as no button)
ENDPROC
}

PROCEDURE(add_combo)
{
    if (widget_count >= MAX_WIDGETS) {
        // Handle error - too many buttons
        RETURNINTX(-1);
    }

    // Get parameters
    int x = GETINT(ARG1);
    int y = GETINT(ARG2);
    
    // Create combo box
    GtkWidget *combo = gtk_combo_box_text_new();
    
    // Store token with the combo
     g_object_set_data(G_OBJECT(combo), "combo-token", GINT_TO_POINTER(widget_count + 1));
    
    // Connect changed signal
    g_signal_connect(combo, "changed", G_CALLBACK(combo_changed_callback), NULL);
    
 // Add items from the comma-separated list in ARG0
    int items=GETARRAYHI(ARG0);
    for(int i=0;i<items;i++) {
        char * item = GETSARRAY(ARG0, i);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), item);
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_fixed_put(GTK_FIXED(main_fixed), combo, x, y);
    gtk_widget_show(combo);


    // Store the widget address
    if (widget_count < MAX_WIDGETS) {
        widgets[widget_count++] = combo;
    }
    RETURNINT(widget_count);  // Return the index+1 (to have number >0)  of the widget in the widgets array
ENDPROC
}

PROCEDURE(combo_add_item)
{
    int index = GETINT(ARG0) - 1;
    const char *text = GETSTRING(ARG1);

    if (widget_count >= MAX_WIDGETS) {
        // Handle error - too many buttons
        RETURNINTX(-1);
    }

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widgets[index]), text);
    RETURNINT(1);
ENDPROC
}

PROCEDURE(combo_remove_item)
{
    int index = GETINT(ARG0) - 1;
    int position = GETINT(ARG1);

    if (widget_count >= MAX_WIDGETS) {
        // Handle error - too many buttons
        RETURNINTX(-1);
    }

    gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(widgets[index]), position);
    RETURNINT(1);
ENDPROC
}

PROCEDURE(combo_clear)
{
    int index = GETINT(ARG0) - 1;

    if (widget_count >= MAX_WIDGETS) {
        // Handle error - too many buttons
        RETURNINTX(-1);
    }

    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(widgets[index]));
    RETURNINT(1);
ENDPROC
}

PROCEDURE(get_combo_index)
{
    int index = GETINT(ARG0) - 1;

    if (widget_count >= MAX_WIDGETS) {
        // Handle error - too many buttons
        RETURNINTX(-1);
    }

    int active = gtk_combo_box_get_active(GTK_COMBO_BOX(widgets[index]));
    RETURNINT(active);
ENDPROC
}

PROCEDURE(cleanup_gui)
{
    // Clean up widgets first
    if (main_window) {
        gtk_widget_destroy(main_window);
        main_window = NULL;
    }
    
    // Clean up stored widgets
    for (int i = 0; i < widget_count; i++) {
        widgets[i] = NULL;
    }

    
    // Release our held references
    if (gtk_settings) {
        g_object_unref(gtk_settings);
        gtk_settings = NULL;
    }
    if (gdk_display) {
        g_object_unref(gdk_display);
        gdk_display = NULL;
    }
    widget_count=0;
ENDPROC
}

PROCEDURE(add_list)
{
    if (widget_count >= MAX_WIDGETS) {
        // Handle error - too many buttons
        RETURNINTX(-1);
    }

    // Get parameters
    int x = GETINT(ARG0);
    int y = GETINT(ARG1);
    int width = GETINT(ARG2);
    int height = GETINT(ARG3);
    
    // Create scrolled window container
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, width, height);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
    
    // Create list box
    GtkWidget *list = gtk_list_box_new();
    
    // Store token with the list
    g_object_set_data(G_OBJECT(list), "list-token", GINT_TO_POINTER(widget_count + 1));
    
    // Connect selection signal
    g_signal_connect(list, "row-selected", G_CALLBACK(list_row_selected), NULL);
    
    // Add list to scrolled window
    gtk_container_add(GTK_CONTAINER(scrolled), list);
    
    // Add to fixed container
    gtk_fixed_put(GTK_FIXED(main_fixed), scrolled, x, y);
    gtk_widget_show_all(scrolled);
    
    if (widget_count < MAX_WIDGETS) {
        widgets[widget_count++] = list;
    }

    RETURNINT(widget_count);  // Return the index+1 (to have number >0)  of the widget in the widgets array
ENDPROC
}

PROCEDURE(list_add_item)
{
    int index = GETINT(ARG0) - 1;
    const char *text = GETSTRING(ARG1);
    const char *bg_color = GETSTRING(ARG2);  // New background color parameter
    
    if (index < 0 || index >= widget_count) {
        RETURNINT(0);  // Invalid index
    }
    
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    
    // Create a new row
    GtkWidget *row = gtk_list_box_row_new();
    gtk_container_add(GTK_CONTAINER(row), label);
    
    // Set background color if provided
    set_widget_background(row, bg_color);
    
    // Insert the row into the list
    gtk_list_box_insert(GTK_LIST_BOX(widgets[index]), row, -1);
    gtk_widget_show_all(row);
    
    RETURNINT(1);
ENDPROC
}

PROCEDURE(list_clear)
{
    int index = GETINT(ARG0) - 1;
    
    if (index < 0 || index >= widget_count) {
        RETURNINT(0);  // Invalid index
    }
    
    GtkListBox *list = GTK_LIST_BOX(widgets[index]);
    GtkListBoxRow *row;
    while ((row = gtk_list_box_get_row_at_index(list, 0)) != NULL) {
        gtk_container_remove(GTK_CONTAINER(list), GTK_WIDGET(row));
    }
    
    RETURNINT(1);
ENDPROC
}

PROCEDURE(list_get_selected)
{
    int index = GETINT(ARG0) - 1;

    if (index < 0 || index >= widget_count) {
        RETURNINT(-1);  // Invalid index
    }
    
    GtkListBox *list = GTK_LIST_BOX(widgets[index]);
    GtkListBoxRow *row = gtk_list_box_get_selected_row(list);
    if (!row) {
        RETURNINT(-1);  // No selection
    }
    
    RETURNINT(gtk_list_box_row_get_index(row));
ENDPROC
}

PROCEDURE(list_get_selected_item) {
    int index = GETINT(ARG0) - 1;

    if (index < 0 || index >= widget_count) {
        RETURNSTRX("");  // Invalid index
    }

    GtkListBox *list = GTK_LIST_BOX(widgets[index]);
    GtkListBoxRow *row = gtk_list_box_get_selected_row(list);
    if (!row) {
        RETURNSTRX("");  // No selection
    }
// take the label from the row
    GtkWidget *label = gtk_bin_get_child(GTK_BIN(row));
    const char *text = gtk_label_get_text(GTK_LABEL(label));
    RETURNSTRX(text);  // Return the text of the label
 ENDPROC
}

PROCEDURE(list_set_header)
{
    int index = GETINT(ARG0)-1;
    const char *header_text = GETSTRING(ARG1);
    const char *text_color = GETSTRING(ARG2);
    const char *bg_color = GETSTRING(ARG3);  // New background color parameter
    
    if (index < 0 || index >= widget_count) {
        RETURNINT(0);  // Invalid index
    }
    
    // Create header row
    GtkWidget *header_label = gtk_label_new(NULL);
    
    // Create markup with text color
    char *markup;
    if (text_color && *text_color) {
        markup = g_markup_printf_escaped("<span color='%s'><b>%s</b></span>", 
                                       text_color, header_text);
    } else {
        markup = g_markup_printf_escaped("<b>%s</b>", header_text);
    }
    
    gtk_label_set_markup(GTK_LABEL(header_label), markup);
    g_free(markup);
    
    gtk_widget_set_halign(header_label, GTK_ALIGN_START);
    
    GtkWidget *header_row = gtk_list_box_row_new();
    gtk_container_add(GTK_CONTAINER(header_row), header_label);
    
    // Set background color if provided
    set_widget_background(header_row, bg_color);
    
    // Make header non-selectable
    gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(header_row), FALSE);
    
    // Add header as first row
    gtk_list_box_insert(GTK_LIST_BOX(widgets[index]), header_row, 0);
    gtk_widget_show_all(header_row);
    
    RETURNINT(1);
ENDPROC
}

PROCEDURE(add_edit)
{
    int x = GETINT(ARG0);
    int y = GETINT(ARG1);
    const char *placeholder = GETSTRING(ARG2);  // Placeholder text

    // Create the edit (entry) widget
    GtkWidget *edit = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(edit), placeholder);
    
    // Place the edit widget in the fixed container
    gtk_fixed_put(GTK_FIXED(main_fixed), edit, x, y);
    gtk_widget_show(edit);

    if (widget_count < MAX_WIDGETS) {
        widgets[widget_count++] = edit;
    }

    RETURNINT(widget_count);  // Indicate success
ENDPROC
}

PROCEDURE(get_widget_address)
{
    int index = GETINT(ARG0) - 1;  // Convert to zero-based index
    
    if (index < 0 || index >= widget_count) {
        RETURNINT(0);  // Invalid index
    }
    
    // Return the address of the widget
    RETURNINT(widget_count);  // Cast to int for REXX
ENDPROC
}

LOADFUNCS
    ADDPROC(init_window, "gui.init_window", "b", ".int", "title=.string, width=.int,height=.int");
    ADDPROC(add_button, "gui.add_button", "b", ".int", "button_text=.string,x=.int,y=.int");
    ADDPROC(add_text, "gui.add_text", "b", ".int", "text=.string,x=.int,y=.int");
    ADDPROC(show_window, "gui.show_window", "b", ".int", "");
    ADDPROC(process_events, "gui.process_events", "b", ".int", "timeout=.int");
    ADDPROC(add_combo, "gui.add_combo", "b", ".int", "expose items=.string[],x=.int,y=.int");
    ADDPROC(combo_add_item, "gui.combo_add_item", "b", ".int", "combo=.int,text=.string");
    ADDPROC(combo_remove_item, "gui.combo_remove_item", "b", ".int", "combo=.int,position=.int");
    ADDPROC(combo_clear, "gui.combo_clear", "b", ".int", "combo=.int");
    ADDPROC(get_combo_index, "gui.get_combo_index", "b", ".int", "combo=.int");
    ADDPROC(cleanup_gui, "gui.cleanup", "b", ".void", "");
    ADDPROC(add_list, "gui.add_list", "b", ".int", "x=.int,y=.int,width=.int,height=.int");
    ADDPROC(list_add_item, "gui.list_add_item", "b", ".int", "list=.int,text=.string,bg_color=.string");
    ADDPROC(list_clear, "gui.list_clear", "b", ".int", "list=.int");
    ADDPROC(list_get_selected, "gui.list_get_selected", "b", ".int", "list=.int");
    ADDPROC(list_get_selected_item, "gui.list_get_item", "b", ".string", "list=.int");
    ADDPROC(list_set_header, "gui.list_header", "b", ".int", "list=.int,header=.string,text_color=.string,bg_color=.string");
    ADDPROC(add_edit, "gui.add_edit", "b", ".int", "x=.int,y=.int,placeholder=.string");
    ADDPROC(get_widget_address, "gui.get_widget_address", "b", ".int", "index=.int");
    ADDPROC(rgb_to_hex, "gui.rgb_to_hex", "b", ".string", "r=.int,g=.int,b=.int");
ENDLOADFUNCS