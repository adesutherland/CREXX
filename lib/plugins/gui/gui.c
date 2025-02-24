//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <gtk/gtk.h>
#include <glib.h>     // Add this for g_main_context functions
#include <gio/gio.h>
#include <math.h>
#include <cairo.h> // Ensure you include the necessary headers

// Function prototype for copy_file
gboolean copy_file(const char *source_path, const char *destination_path, GError **error);

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

// Declare the TextView and TextBuffer globally or in the relevant function
GtkWidget *output_text_view;
GtkTextBuffer *output_text_buffer;

// Function to initialize the output widget
void init_output_widget(GtkWidget *parent) {
    // Create a new TextView and TextBuffer
    output_text_view = gtk_text_view_new();
    output_text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_text_view));

    // Set properties for the TextView
    gtk_text_view_set_editable(GTK_TEXT_VIEW(output_text_view), FALSE);  // Make it read-only
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(output_text_view), GTK_WRAP_WORD);  // Wrap text

    // Add the TextView to the parent container
    gtk_box_pack_start(GTK_BOX(parent), output_text_view, TRUE, TRUE, 0);
}

// Function to append text to the output widget
void append_to_output(const gchar *text) {
    gtk_text_buffer_insert_at_cursor(output_text_buffer, text, -1);  // Append text to the buffer
    gtk_text_buffer_insert_at_cursor(output_text_buffer, "\n", -1);  // Add a new line
}

static gboolean button_clicked(GtkWidget *widget, gpointer data) {
    // Find the index of the clicked widget in the widgets array
    for (int i = 0; i < widget_count; i++) {
        if (widgets[i] == widget) {  // Compare addresses
            last_event_token = i + 1;  // Store the index (1-based)
            break;
        }
    }
    user_event_occurred = TRUE;
    // g_print("Button clicked! Token: %d\n", last_event_token);  // Debugging output
    return FALSE;  // Allow event propagation
}

static gboolean timeout_callback(gpointer data) {
    return FALSE;  // Return FALSE to remove the source
}

static gboolean window_close_callback(GtkWidget *widget, GdkEvent *event, gpointer data) {
    user_event_occurred = TRUE;
    last_event_token = WINDOW_CLOSE_TOKEN;  // Set a special token for window close
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
    
    // Create the CSS provider with error handling
    GtkCssProvider *css_provider = gtk_css_provider_new();
    if (css_provider) {
        GError *error = NULL;
        const char *css_data = 
            ".title { font-size: 24px; font-weight: bold; }"
            ".message { font-size: 20px; }";
            
        gtk_css_provider_load_from_data(css_provider, css_data, -1, &error);
        
        if (error == NULL) {
            // Apply the CSS provider to the default screen
            GdkScreen *screen = gdk_screen_get_default();
            if (screen) {
                gtk_style_context_add_provider_for_screen(screen,
                    GTK_STYLE_PROVIDER(css_provider),
                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            }
        } else {
            g_error_free(error);
        }
        
        g_object_unref(css_provider);  // Release our reference
    }
    
    // Create main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), GETSTRING(ARG0));
    gtk_window_set_default_size(GTK_WINDOW(main_window), GETINT(ARG1), GETINT(ARG2));
    
    // Create a vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(main_window), vbox);
    
    // Create a fixed container and add it to the box
    main_fixed = gtk_fixed_new();
    gtk_box_pack_start(GTK_BOX(vbox), main_fixed, TRUE, TRUE, 0);
    
    // Connect delete-event signal
    g_signal_connect(main_window, "delete-event", G_CALLBACK(window_close_callback), NULL);

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
        RETURNINTX(-1);  // Handle error - too many buttons
    }
    
    const char *button_text = GETSTRING(ARG0);
    int x = GETINT(ARG1);
    int y = GETINT(ARG2);
    
    // Create the button and show it immediately
    widgets[widget_count] = gtk_button_new_with_label(button_text);
    gtk_fixed_put(GTK_FIXED(main_fixed), widgets[widget_count], x, y);
    
    // Store the current token before incrementing
    int current_token = next_event_token;
    
    // Store the token with the button
    g_object_set_data(G_OBJECT(widgets[widget_count]), "button-token", GINT_TO_POINTER(current_token));
    
    g_signal_connect(widgets[widget_count], "clicked", G_CALLBACK(button_clicked), NULL);
    gtk_widget_show(widgets[widget_count]);
    
    widget_count++;
    next_event_token++;  // Increment for the next button
    
    RETURNINT(widget_count);  // Return the current count of widgets
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
    
    // Store the current token before incrementing
    int current_token = next_event_token;
    widget_count++;
    next_event_token++;

    RETURNINT(current_token);  // Return the token that will be used in events
ENDPROC
}


PROCEDURE(show_window) {
    int x = GETINT(ARG0);
    int y = GETINT(ARG1);

    if (main_window) {
        // Initially hide the window
        //    gtk_widget_hide(main_window);

        // If x or y is less than or equal to 0, center the window
        if (x <= 0 && y <= 0) {
            GdkScreen *screen = gdk_screen_get_default();
            int screen_width = gdk_screen_get_width(screen);
            int screen_height = gdk_screen_get_height(screen);
            int window_width, window_height;  // Declare variables to hold the size

            // Get the size of the window
            gtk_window_get_size(GTK_WINDOW(main_window), &window_width, &window_height);

            // Calculate the new position to center the window
            x = (screen_width - window_width) / 2;
            y = (screen_height - window_height) / 2;
        }

        // Set the position of the window
        gtk_window_move(GTK_WINDOW(main_window), x, y);

        // Show the fixed container first
        // gtk_widget_show(main_fixed);
        
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
}

PROCEDURE(process_events) {
    // Get timeout value from argument, default to 250ms if not specified
    int timeout = GETINT(ARG0) * 1000; // Convert milliseconds to microseconds
    int gtk_timeout = 10 * 1000;      // 10ms sleep interval in microseconds
    int waited = 0;                   // Track elapsed time
    if (timeout <= 0) timeout = 250000; // Default timeout is 250ms

    // Reset the event flag
    user_event_occurred = FALSE;

    // Process events and wait for user interaction or timeout
    while (waited < timeout) {
        // Process any pending events
        while (gtk_events_pending()) {
            gtk_main_iteration_do(FALSE);
            if (user_event_occurred) {
                // g_print("Event processed! Widget index: %d\n", last_event_token);  // Debugging output
                RETURNINTX(last_event_token); // Return the index of the widget that triggered the event
            }
        }

        // If no events are processed, wait for a short period
        g_usleep(gtk_timeout);
        waited += gtk_timeout;
    }

    RETURNINTX(0); // Timeout occurred (using 0 as no button)
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
        RETURNINTX(-1);  // No selection
    }
    
    RETURNINTX(gtk_list_box_row_get_index(row));
ENDPROC
}

PROCEDURE(list_get_selected_item) {
    int index = GETINT(ARG0) - 1;  // Convert to zero-based index

    if (index < 0 || index >= widget_count) {
        RETURNSTRX("");  // Invalid index
    }

    GtkListBox *list = GTK_LIST_BOX(widgets[index]);
    GtkListBoxRow *selected_row = gtk_list_box_get_selected_row(list);

    if (selected_row) {
        GtkWidget *label = gtk_bin_get_child(GTK_BIN(selected_row));
        const char *text = gtk_label_get_text(GTK_LABEL(label));
        RETURNSTRX(text);  // Return the text of the selected item
    }

    RETURNSTRX("");  // No item selected
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

PROCEDURE(add_edit) {
    int x = GETINT(ARG0);
    int y = GETINT(ARG1);
    int width = GETINT(ARG2);  // New width parameter

    // Create the edit (entry) widget
    GtkWidget *edit = gtk_entry_new();

    // Set the width of the entry widget
    gtk_widget_set_size_request(edit, width, -1);  // Set width, height is -1 for default

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
    RETURNINT((intptr_t)widgets[index]);  // Cast widget pointer to integer
ENDPROC
}

PROCEDURE(set_text) {
    int index = GETINT(ARG0)-1;
    const char *text = GETSTRING(ARG1);
    
    // Find the widget with this token
    if (widgets[index] && GTK_IS_LABEL(widgets[index])) {
        gtk_label_set_text(GTK_LABEL(widgets[index]), text);
        RETURNINTX(1);
    }
    RETURNINT(-1);  // Not found or not a label
ENDPROC
}

PROCEDURE(add_message_area) {
    int x = GETINT(ARG0);        // X position
    int y = GETINT(ARG1);        // Y position
    int width = GETINT(ARG2);    // Width of the message area
    int height = GETINT(ARG3);   // Height of the message area

    // Create a scrolled window to contain the text view
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled_window, width, height);
    gtk_fixed_put(GTK_FIXED(main_fixed), scrolled_window, x, y);

    // Create the text view
    GtkWidget *text_view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(text_view), FALSE);  // Read-only
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    // Store the widget
    widgets[widget_count] = text_view;
    int current_widget = widget_count;
    widget_count++;

    // Show the widgets
    gtk_widget_show(text_view);
    gtk_widget_show(scrolled_window);

    RETURNINT(current_widget);
ENDPROC
}

PROCEDURE(append_message) {
    int widget_index = GETINT(ARG0);       // Widget index
    const char *message = GETSTRING(ARG1);  // Message to append

    // Get the text view
    GtkWidget *text_view = widgets[widget_index];
    if (!text_view || !GTK_IS_TEXT_VIEW(text_view)) {
        RETURNINT(-1);  // Invalid widget index or not a text view
    }

    // Get the buffer
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    
    // Get end iterator
    GtkTextIter end;
    gtk_text_buffer_get_end_iter(buffer, &end);
    
    // Insert the text at the end
    gtk_text_buffer_insert(buffer, &end, message, -1);
    gtk_text_buffer_insert(buffer, &end, "\n", -1);  // Add newline

    // Scroll to the end
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(text_view), &end, 0.0, FALSE, 0.0, 0.0);

    RETURNINT(0);
ENDPROC
}

PROCEDURE(add_status_bar) {
    if (widget_count >= MAX_WIDGETS) {
        RETURNINTX(-1);
    }
    
    // Create a frame for the status bar
    GtkWidget *frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    
    // Create status bar
    GtkWidget *status_bar = gtk_statusbar_new();
    
    // Remove internal padding of the status bar
    gtk_widget_set_margin_top(status_bar, 0);
    gtk_widget_set_margin_bottom(status_bar, 0);
    gtk_widget_set_margin_start(status_bar, 2);
    gtk_widget_set_margin_end(status_bar, 2);
    
    // Set size request for the status bar
    gtk_widget_set_size_request(status_bar, -1, 15);  // Try 15 pixels height
    
    // Add status bar to frame
    gtk_container_add(GTK_CONTAINER(frame), status_bar);
    
    // Add frame to main window (not to fixed container)
    gtk_box_pack_end(GTK_BOX(gtk_bin_get_child(GTK_BIN(main_window))), 
                     frame, 
                     FALSE, FALSE, 0);  // Changed to FALSE, FALSE and 0 padding
    
    // Show all widgets
    gtk_widget_show_all(frame);
    
    // Store the status bar widget (not the frame)
    widgets[widget_count] = status_bar;
    
    // Store the current token before incrementing
    int current_token = next_event_token;
    widget_count++;
    next_event_token++;
    
    // Push an initial empty message to create the space
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), 0, "");
    
    RETURNINT(current_token);
ENDPROC
}

PROCEDURE(set_status) {
    int index = GETINT(ARG0);
    const char *message = GETSTRING(ARG1);
    
    // Find the status bar widget
    for (int i = 0; i < widget_count; i++) {
        if (widgets[i] && GTK_IS_STATUSBAR(widgets[i])) {
            gtk_statusbar_pop(GTK_STATUSBAR(widgets[i]), 0);  // Remove previous message
            gtk_statusbar_push(GTK_STATUSBAR(widgets[i]), 0, message);
            RETURNINT(1);
        }
    }
    
    RETURNINT(0);
ENDPROC
}

PROCEDURE(list_get_count) {
    int index = GETINT(ARG0) - 1;  // Convert to zero-based index

    if (index < 0 || index >= widget_count) {
        RETURNINT(0);  // Invalid index
    }

    GtkListBox *list = GTK_LIST_BOX(widgets[index]);

    // Count the number of rows in the list
    int row_count = 0;
    GList *children = gtk_container_get_children(GTK_CONTAINER(list));
    for (GList *iter = children; iter != NULL; iter = iter->next) {
        row_count++;
    }
    g_list_free(children);  // Free the list of children

    RETURNINT(row_count);  // Return the number of items in the list
ENDPROC
}

// =================================================================================================================
// Pick Service temporary integrated in GUI plugin
// =================================================================================================================

static gboolean is_valid_utf8(const char *str) {
    if (!str) return TRUE;
    const guchar *bytes = (const guchar *)str;

    while (*bytes) {
        if (bytes[0] < 0x80) {
            // Valid ASCII
            bytes++;
        } else if (bytes[0] < 0xC0) {
            // Invalid UTF-8 sequence
            return FALSE;
        } else if (bytes[0] < 0xE0) {
            // 2-byte sequence
            if ((bytes[1] & 0xC0) != 0x80) return FALSE;
            bytes += 2;
        } else if (bytes[0] < 0xF0) {
            // 3-byte sequence
            if ((bytes[1] & 0xC0) != 0x80 ||
                (bytes[2] & 0xC0) != 0x80) return FALSE;
            bytes += 3;
        } else if (bytes[0] < 0xF5) {
            // 4-byte sequence
            if ((bytes[1] & 0xC0) != 0x80 ||
                (bytes[2] & 0xC0) != 0x80 ||
                (bytes[3] & 0xC0) != 0x80) return FALSE;
            bytes += 4;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}

static char* sanitize_utf8(const char *input) {
    if (!input) return g_strdup("");
    if (is_valid_utf8(input)) return g_strdup(input);

    // Convert to valid UTF-8 or replace invalid chars
    GString *result = g_string_new(NULL);
    const guchar *bytes = (const guchar *)input;

    while (*bytes) {
        if (is_valid_utf8((const char *)bytes)) {
            g_string_append_c(result, *bytes);
            bytes++;
        } else {
            g_string_append_c(result, '*');  // Replace invalid char
            bytes++;
        }
    }

    // Get the string and free the GString structure properly
    char *sanitized = g_strdup(result->str);  // Make a copy of the string
    g_string_free(result, TRUE);  // TRUE to free both the structure and the string data
    return sanitized;
}

static gboolean folder_filter(const GtkFileFilterInfo *filter_info, gpointer data) {
    return g_file_test(filter_info->filename, G_FILE_TEST_IS_DIR);
}


PROCEDURE(file_pick)
{
    static int gtk_initialized = 0;

    // Initialize GTK if not already done
    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");  // GTK initialization failed
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *initial_dir = GETSTRING(ARG1);
    int save_dialog = GETINT(ARG2);
    const char *filter_patterns = GETSTRING(ARG3);
    const char *default_name = GETSTRING(ARG4);
    int allow_multiple = GETINT(ARG5);
    int select_folder = GETINT(ARG6);

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);  // Make sure it doesn't get destroyed

    // Create the file chooser dialog
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
            title,
            GTK_WINDOW(temp_parent),
            select_folder ? GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER :
            (save_dialog ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN),
            "_Cancel", GTK_RESPONSE_CANCEL,
            save_dialog ? "_Save" : (select_folder ? "_Select" : "_Open"), GTK_RESPONSE_ACCEPT,
            NULL);

    // Set dialog size and make it modal
    gtk_window_set_default_size(GTK_WINDOW(dialog), 800, 600);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    // Set up file chooser properties
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    if (save_dialog) {
        gtk_file_chooser_set_do_overwrite_confirmation(chooser, TRUE);
    }

    if (allow_multiple && !save_dialog && !select_folder) {
        gtk_file_chooser_set_select_multiple(chooser, TRUE);
    }

    if (initial_dir && *initial_dir) {
        gtk_file_chooser_set_current_folder(chooser, initial_dir);
    }

    if (save_dialog && default_name && *default_name) {
        char *safe_default_name = sanitize_utf8(default_name);
        gtk_file_chooser_set_current_name(chooser, safe_default_name);
        g_free(safe_default_name);
    }

    // Add filters if specified
    if (filter_patterns && *filter_patterns) {
        char *patterns = strdup(filter_patterns);
        char *pattern = strtok(patterns, ",");
        GtkFileFilter *filter = gtk_file_filter_new();

        while (pattern != NULL) {
            while (*pattern == ' ') pattern++;
            char *end = pattern + strlen(pattern) - 1;
            while (end > pattern && *end == ' ') end--;
            *(end + 1) = '\0';

            gtk_file_filter_add_pattern(filter, pattern);
            pattern = strtok(NULL, ",");
        }

        gtk_file_filter_set_name(filter, filter_patterns);
        gtk_file_chooser_add_filter(chooser, filter);

        // Add "All files" filter
        GtkFileFilter *all_filter = gtk_file_filter_new();
        gtk_file_filter_set_name(all_filter, "All files");
        gtk_file_filter_add_pattern(all_filter, "*");
        gtk_file_chooser_add_filter(chooser, all_filter);

        free(patterns);
    }

    // Show the dialog
    gtk_widget_show_all(dialog);

    // Run dialog and get result
    char result[32768] = "";
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        if (allow_multiple && !save_dialog && !select_folder) {
            GSList *filenames = gtk_file_chooser_get_filenames(chooser);
            GSList *iter;
            for (iter = filenames; iter != NULL; iter = iter->next) {
                if (result[0]) strcat(result, "|");
                strcat(result, (char*)iter->data);
                g_free(iter->data);
            }
            g_slist_free(filenames);
        } else {
            char *filename = gtk_file_chooser_get_filename(chooser);
            if (filename) {
                strcpy(result, filename);
                g_free(filename);
            }
        }
    }

    // Clean up
    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    // Process any pending events
    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(path_pick)
{
    static int gtk_initialized = 0;

    // Initialize GTK if not already done
    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");  // GTK initialization failed
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *initial_dir = GETSTRING(ARG1);

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create the folder chooser dialog
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
            title,
            GTK_WINDOW(temp_parent),
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Select", GTK_RESPONSE_ACCEPT,
            NULL);

    // Set dialog size and make it modal
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 400);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    // Set up chooser properties
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    // Create and set folder-only filter
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_custom(filter,
                               GTK_FILE_FILTER_FILENAME,
                               folder_filter,
                               NULL, NULL);
    gtk_file_chooser_set_filter(chooser, filter);

    // Additional folder-only settings
    gtk_file_chooser_set_action(chooser, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    g_object_set(G_OBJECT(dialog),
                 "show-hidden", FALSE,
                 "select-multiple", FALSE,
                 NULL);

    // Set initial directory if specified
    if (initial_dir && *initial_dir) {
        gtk_file_chooser_set_current_folder(chooser, initial_dir);
    }

    // Show the dialog
    gtk_widget_show_all(dialog);

    // Run dialog and get result
    char result[32768] = "";
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *path = gtk_file_chooser_get_filename(chooser);
        if (path) {
            strcpy(result, path);
            g_free(path);
        }
    }

    // Clean up
    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    // Process any pending events
    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(date_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    int show_time = GETINT(ARG1);      // 0 = date only, 1 = date and time
    const char *format = GETSTRING(ARG2);  // Output format (strftime style)

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    char *safe_title = sanitize_utf8(title);
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            safe_title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Today", GTK_RESPONSE_APPLY,  // Added Today button
            "_OK", GTK_RESPONSE_OK,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, show_time ? 250 : 200);

    // Create calendar with week numbers
    GtkWidget *calendar = gtk_calendar_new();
    gtk_calendar_set_display_options(GTK_CALENDAR(calendar),
                                     GTK_CALENDAR_SHOW_WEEK_NUMBERS |
                                     GTK_CALENDAR_SHOW_DAY_NAMES |
                                     GTK_CALENDAR_SHOW_HEADING);

    // Time widgets
    GtkWidget *hour_spin = NULL;
    GtkWidget *minute_spin = NULL;
    GtkWidget *second_spin = NULL;
    GtkWidget *ampm_combo = NULL;

    if (show_time) {
        GtkWidget *time_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

        // Hour spinner (1-12)
        hour_spin = gtk_spin_button_new_with_range(1, 12, 1);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(hour_spin), 12);

        // Minute spinner (0-59)
        minute_spin = gtk_spin_button_new_with_range(0, 59, 1);

        // Second spinner (0-59)
        second_spin = gtk_spin_button_new_with_range(0, 59, 1);

        // AM/PM combo
        ampm_combo = gtk_combo_box_text_new();
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ampm_combo), "AM");
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(ampm_combo), "PM");
        gtk_combo_box_set_active(GTK_COMBO_BOX(ampm_combo), 0);

        // Add time widgets with labels
        GtkWidget *hour_label = gtk_label_new("Hour:");
        GtkWidget *minute_label = gtk_label_new("Min:");
        GtkWidget *second_label = gtk_label_new("Sec:");

        gtk_box_pack_start(GTK_BOX(time_box), hour_label, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(time_box), hour_spin, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(time_box), minute_label, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(time_box), minute_spin, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(time_box), second_label, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(time_box), second_spin, FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(time_box), ampm_combo, FALSE, FALSE, 5);

        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                           time_box, FALSE, FALSE, 5);
    }

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       calendar, TRUE, TRUE, 5);

    gtk_widget_show_all(dialog);

    // Handle dialog response
    char result[256] = "";
    int response;
    while ((response = gtk_dialog_run(GTK_DIALOG(dialog))) != GTK_RESPONSE_CANCEL) {
        if (response == GTK_RESPONSE_APPLY) {  // Today button
            time_t now = time(NULL);
            struct tm *today = localtime(&now);
            gtk_calendar_select_month(GTK_CALENDAR(calendar), today->tm_mon, today->tm_year + 1900);
            gtk_calendar_select_day(GTK_CALENDAR(calendar), today->tm_mday);
            if (show_time) {
                int hour = today->tm_hour;
                gboolean is_pm = hour >= 12;
                hour = hour % 12;
                if (hour == 0) hour = 12;

                gtk_spin_button_set_value(GTK_SPIN_BUTTON(hour_spin), hour);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(minute_spin), today->tm_min);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(second_spin), today->tm_sec);
                gtk_combo_box_set_active(GTK_COMBO_BOX(ampm_combo), is_pm ? 1 : 0);
            }
            continue;  // Keep dialog open
        }

        if (response == GTK_RESPONSE_OK) {
            guint year, month, day;
            gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
            month++; // GTK months are 0-based

            struct tm time_info = {0};
            time_info.tm_year = year - 1900;
            time_info.tm_mon = month - 1;
            time_info.tm_mday = day;

            if (show_time) {
                int hour = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(hour_spin));
                int is_pm = gtk_combo_box_get_active(GTK_COMBO_BOX(ampm_combo));

                // Convert to 24-hour
                if (is_pm && hour < 12) hour += 12;
                if (!is_pm && hour == 12) hour = 0;

                time_info.tm_hour = hour;
                time_info.tm_min = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(minute_spin));
                time_info.tm_sec = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(second_spin));
            }

            const char *actual_format = format && *format ? format :
                                        (show_time ? "%Y-%m-%d %I:%M:%S %p" : "%Y-%m-%d");
            strftime(result, sizeof(result), actual_format, &time_info);
        }
        break;
    }

    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(list_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    int multi_select = GETINT(ARG2);         // 0=single selection, 1=multiple selection
    const char *message = GETSTRING(ARG3);   // Optional message above list

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_OK", GTK_RESPONSE_OK,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 400);

    // Create message label if provided
    if (message && *message) {
        char *safe_message = sanitize_utf8(message);
        GtkWidget *label = gtk_label_new(safe_message);
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                           label, FALSE, FALSE, 5);
        g_free(safe_message);
    }

    // Create scrolled window for list
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);

    // Create list box
    GtkWidget *list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box),
                                    multi_select ? GTK_SELECTION_MULTIPLE :
                                    GTK_SELECTION_SINGLE);

    // Add items to list
    int items=GETARRAYHI(ARG1);
    for(int i=0;i<items;i++) {
        char *item = GETSARRAY(ARG1, i);
        char *safe_item = sanitize_utf8(item);
        GtkWidget *row_label = gtk_label_new(safe_item);
        gtk_widget_set_halign(row_label, GTK_ALIGN_START);
        gtk_widget_set_margin_start(row_label, 5);
        gtk_widget_set_margin_end(row_label, 5);
        gtk_widget_set_margin_top(row_label, 2);
        gtk_widget_set_margin_bottom(row_label, 2);

        GtkWidget *row = gtk_list_box_row_new();
        gtk_container_add(GTK_CONTAINER(row), row_label);
        gtk_list_box_insert(GTK_LIST_BOX(list_box), row, -1);
        g_free(safe_item);
    }

    // Add list to scrolled window
    gtk_container_add(GTK_CONTAINER(scrolled), list_box);

    // Add scrolled window to dialog
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       scrolled, TRUE, TRUE, 5);

    gtk_widget_show_all(dialog);

    // Run dialog and get result
    char result[32768] = "";
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        GList *selected = gtk_list_box_get_selected_rows(GTK_LIST_BOX(list_box));
        GList *iter;
        for (iter = selected; iter != NULL; iter = iter->next) {
            GtkWidget *row = GTK_WIDGET(iter->data);
            GtkWidget *label = gtk_bin_get_child(GTK_BIN(row));
            const char *text = gtk_label_get_text(GTK_LABEL(label));

            if (result[0]) strcat(result, "|");
            strcat(result, text);
        }
        g_list_free(selected);
    }

    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(dialog_pick)
{
    static int gtk_initialized = 0;

    // Initialize GTK if not already done
    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");  // GTK initialization failed
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);
    const char *buttons = GETSTRING(ARG2);  // Format: "button1|button2|button3"

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create the dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            NULL  // We'll add buttons dynamically
    );

    // Add message label
    if (message && *message) {
        char *safe_message = sanitize_utf8(message);
        GtkWidget *label = gtk_label_new(safe_message);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_label_set_max_width_chars(GTK_LABEL(label), 50);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);
        gtk_widget_set_margin_top(label, 10);
        gtk_widget_set_margin_bottom(label, 10);

        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                           label, TRUE, TRUE, 0);
        g_free(safe_message);
    }

    // Add buttons dynamically
    if (buttons && *buttons) {
        char *buttons_copy = strdup(buttons);
        char *button = strtok(buttons_copy, "|");
        int response_id = 1;  // Start with 1 for custom responses

        while (button != NULL) {
            // Trim whitespace
            while (*button == ' ') button++;
            char *end = button + strlen(button) - 1;
            while (end > button && *end == ' ') end--;
            *(end + 1) = '\0';

            gtk_dialog_add_button(GTK_DIALOG(dialog), button, response_id);
            button = strtok(NULL, "|");
            response_id++;
        }

        free(buttons_copy);
    } else {
        // Default OK button if none specified
        gtk_dialog_add_button(GTK_DIALOG(dialog), "_OK", 1);
    }

    // Set dialog properties
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);
    gtk_widget_show_all(dialog);

    // Run dialog and get result
    int response = gtk_dialog_run(GTK_DIALOG(dialog));

    // Clean up
    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    // Process any pending events
    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    // Convert response to button index (1-based) or empty string if cancelled
    char result[32];
    if (response > 0) {
        snprintf(result, sizeof(result), "%d", response);
    } else {
        result[0] = '\0';
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(input_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);
    const char *default_value = GETSTRING(ARG2);
    int password_mode = GETINT(ARG3);

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_OK", GTK_RESPONSE_OK,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 150);

    // Create message label
    if (message && *message) {
        char *safe_message = sanitize_utf8(message);
        GtkWidget *label = gtk_label_new(safe_message);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);
        gtk_widget_set_margin_top(label, 10);
        gtk_widget_set_margin_bottom(label, 5);
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                           label, FALSE, FALSE, 0);
        g_free(safe_message);
    }

    // Create entry field
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(entry), !password_mode);
    if (password_mode) {
        gtk_entry_set_invisible_char(GTK_ENTRY(entry), '?');
    }
    if (default_value && *default_value) {
        char *safe_value = sanitize_utf8(default_value);
        gtk_entry_set_text(GTK_ENTRY(entry), safe_value);
        gtk_editable_select_region(GTK_EDITABLE(entry), 0, -1);
        g_free(safe_value);
    }
    gtk_widget_set_margin_start(entry, 10);
    gtk_widget_set_margin_end(entry, 10);
    gtk_widget_set_margin_bottom(entry, 10);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       entry, FALSE, FALSE, 0);

    gtk_widget_show_all(dialog);
    gtk_widget_grab_focus(entry);

    // Run dialog and get result
    char result[32768] = "";
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
        if (text) {
            strncpy(result, text, sizeof(result) - 1);
        }
    }

    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(form_pick) {
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_OK", GTK_RESPONSE_OK,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, -1);

    // Create message label if provided
    if (message && *message) {
        char *safe_message = sanitize_utf8(message);
        GtkWidget *label = gtk_label_new(safe_message);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);
        gtk_widget_set_margin_top(label, 10);
        gtk_widget_set_margin_bottom(label, 10);
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                           label, FALSE, FALSE, 0);
        g_free(safe_message);
    }

    // Create grid for form fields
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_widget_set_margin_start(grid, 10);
    gtk_widget_set_margin_end(grid, 10);
    gtk_widget_set_margin_bottom(grid, 10);

    // Get number of fields and create entries
    int field_count = GETARRAYHI(ARG2);
    GtkWidget **entries = malloc(field_count * sizeof(GtkWidget *));

    for (int i = 0; i < field_count; i++) {
        // Create label
        char *label_text = GETSARRAY(ARG2, i);
        char *safe_label = sanitize_utf8(label_text);
        GtkWidget *label = gtk_label_new(safe_label);
        gtk_widget_set_halign(label, GTK_ALIGN_START);

        // Create entry
        entries[i] = gtk_entry_new();

        // Get default value if provided
        if (i < GETARRAYHI(ARG3)) {
            char *default_value = GETSARRAY(ARG3, i);
            if (default_value && *default_value) {
                char *safe_value = sanitize_utf8(default_value);
                gtk_entry_set_text(GTK_ENTRY(entries[i]), safe_value);
                g_free(safe_value);
            }
        }

        // Add to grid
        gtk_grid_attach(GTK_GRID(grid), label, 0, i, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), entries[i], 1, i, 1, 1);
        g_free(safe_label);
    }

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       grid, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);
    gtk_widget_grab_focus(entries[0]);

    // Run dialog and get results
    char result[32768] = "";
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        for (int i = 0; i < field_count; i++) {
            const char *text = gtk_entry_get_text(GTK_ENTRY(entries[i]));
            if (i > 0) strcat(result, "|");
            strcat(result, text ? text : "");
        }
    }

    // Clean up
    free(entries);
    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(notify_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);
    const char *type = GETSTRING(ARG2);  // info, warning, error, success

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_OK", GTK_RESPONSE_OK,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 350, -1);

    // Create box for icon and message
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(hbox, 10);
    gtk_widget_set_margin_end(hbox, 10);
    gtk_widget_set_margin_top(hbox, 10);
    gtk_widget_set_margin_bottom(hbox, 10);

    // Add appropriate icon
    const char *icon_name;
    if (strcasecmp(type, "warning") == 0) {
        icon_name = "dialog-warning";
    } else if (strcasecmp(type, "error") == 0) {
        icon_name = "dialog-error";
    } else if (strcasecmp(type, "success") == 0) {
        icon_name = "emblem-ok";
    } else {  // info or default
        icon_name = "dialog-information";
    }

    GtkWidget *image = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start(GTK_BOX(hbox), image, FALSE, FALSE, 0);

    // Add message with proper styling
    char *safe_message = sanitize_utf8(message);
    GtkWidget *label = gtk_label_new(safe_message);
    g_free(safe_message);

    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(label), 50);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_valign(label, GTK_ALIGN_CENTER);

    // Apply CSS styling based on type
    GtkStyleContext *context = gtk_widget_get_style_context(label);
    GtkCssProvider *provider = gtk_css_provider_new();

    char css_data[256];
    const char *color;

    if (strcasecmp(type, "warning") == 0) {
        color = "#f0a500";  // Orange
    } else if (strcasecmp(type, "error") == 0) {
        color = "#cc0000";  // Red
    } else if (strcasecmp(type, "success") == 0) {
        color = "#00aa00";  // Green
    } else {
        color = "#0066cc";  // Blue (info)
    }

    snprintf(css_data, sizeof(css_data),
             "label { color: %s; font-weight: bold; font-size: 12pt; }",
             color);

    gtk_css_provider_load_from_data(provider, css_data, -1, NULL);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);

    gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, TRUE, 0);

    // Add to dialog
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       hbox, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    // Run dialog
    gtk_dialog_run(GTK_DIALOG(dialog));

    // Clean up
    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX("1");  // Always return "1" since it's just an OK dialog
    ENDPROC
}

PROCEDURE(combo_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_OK", GTK_RESPONSE_OK,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, -1);

    // Create message label if provided
    if (message && *message) {
        char *safe_message = sanitize_utf8(message);
        GtkWidget *label = gtk_label_new(safe_message);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_widget_set_margin_start(label, 10);
        gtk_widget_set_margin_end(label, 10);
        gtk_widget_set_margin_top(label, 10);
        gtk_widget_set_margin_bottom(label, 10);
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                           label, FALSE, FALSE, 0);
        g_free(safe_message);
    }

    // Create combo box
    GtkWidget *combo = gtk_combo_box_text_new();
    gtk_widget_set_margin_start(combo, 10);
    gtk_widget_set_margin_end(combo, 10);
    gtk_widget_set_margin_bottom(combo, 10);

    // Add items to combo
    int items = GETARRAYHI(ARG2);
    for(int i = 0; i < items; i++) {
        char *item = GETSARRAY(ARG2, i);
        char *safe_item = sanitize_utf8(item);
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), safe_item);
        g_free(safe_item);
    }

    // Set default selection if items exist
    if (items > 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    }

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       combo, FALSE, FALSE, 0);

    gtk_widget_show_all(dialog);
    gtk_widget_grab_focus(combo);

    // Run dialog and get result
    char result[32768] = "";
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        char *selected = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
        if (selected) {
            strncpy(result, selected, sizeof(result) - 1);
            g_free(selected);
        }
    }

    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(page_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_OK", GTK_RESPONSE_OK,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);

    // Create notebook (tabbed container)
    GtkWidget *notebook = gtk_notebook_new();
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), TRUE);
    gtk_widget_set_margin_start(notebook, 10);
    gtk_widget_set_margin_end(notebook, 10);
    gtk_widget_set_margin_top(notebook, 10);
    gtk_widget_set_margin_bottom(notebook, 10);

    // Get number of pages
    int page_count = GETARRAYHI(ARG1);  // pages[]
    GtkWidget ***entries = malloc(page_count * sizeof(GtkWidget**));
    int *field_counts = malloc(page_count * sizeof(int));

    // Create pages
    for (int p = 0; p < page_count; p++) {
        // Get page label
        char *page_label = GETSARRAY(ARG2, p);  // labels[]
        char *safe_label = sanitize_utf8(page_label);
        GtkWidget *label = gtk_label_new(safe_label);
        g_free(safe_label);

        // Create grid for this page
        GtkWidget *grid = gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
        gtk_widget_set_margin_start(grid, 10);
        gtk_widget_set_margin_end(grid, 10);
        gtk_widget_set_margin_top(grid, 10);
        gtk_widget_set_margin_bottom(grid, 10);

        // Get fields for this page
        char *fields_str = GETSARRAY(ARG1, p);
        char *fields_copy = strdup(fields_str);
        char *field = strtok(fields_copy, "|");
        int field_count = 0;

        // Count fields first
        while (field != NULL) {
            field_count++;
            field = strtok(NULL, "|");
        }

        // Allocate entries array for this page
        field_counts[p] = field_count;
        entries[p] = malloc(field_count * sizeof(GtkWidget*));

        // Reset for second pass
        free(fields_copy);
        fields_copy = strdup(fields_str);
        field = strtok(fields_copy, "|");
        int row = 0;

        // Create fields
        while (field != NULL) {
            // Create field label
            char *safe_field = sanitize_utf8(field);
            GtkWidget *field_label = gtk_label_new(safe_field);
            g_free(safe_field);
            gtk_widget_set_halign(field_label, GTK_ALIGN_START);

            // Create entry
            entries[p][row] = gtk_entry_new();

            // Get default value if provided
            if (p < GETARRAYHI(ARG3) && row < GETARRAYHI(ARG3)) {
                char *default_value = GETSARRAY(ARG3, p * field_count + row);
                if (default_value && *default_value) {
                    char *safe_value = sanitize_utf8(default_value);
                    gtk_entry_set_text(GTK_ENTRY(entries[p][row]), safe_value);
                    g_free(safe_value);
                }
            }

            // Add to grid
            gtk_grid_attach(GTK_GRID(grid), field_label, 0, row, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), entries[p][row], 1, row, 1, 1);

            field = strtok(NULL, "|");
            row++;
        }

        free(fields_copy);

        // Add page to notebook
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid, label);
    }

    // Add notebook to dialog
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       notebook, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    // Run dialog and get results
    char result[32768] = "";
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        for (int p = 0; p < page_count; p++) {
            if (p > 0) strcat(result, "||");  // Page separator
            for (int f = 0; f < field_counts[p]; f++) {
                if (f > 0) strcat(result, "|");  // Field separator
                const char *text = gtk_entry_get_text(GTK_ENTRY(entries[p][f]));
                strcat(result, text ? text : "");
            }
        }
    }

    // Clean up
    for (int p = 0; p < page_count; p++) {
        free(entries[p]);
    }
    free(entries);
    free(field_counts);

    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(tree_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);
    int item_count = GETARRAYHI(ARG2);  // Number of items to display
    int multi_select = GETINT(ARG4);     // Multi-select option

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_OK", GTK_RESPONSE_OK,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    // Create a tree store
    GtkTreeStore *store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(tree_view), -1, "Item", renderer, "text", 0, NULL);

    // Create a hash table to store iterators
    GHashTable *iter_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                                   (GDestroyNotify)gtk_tree_iter_free);

    // First pass: Create all nodes
    for (int i = 0; i < item_count; i++) {
        char *item_text = GETSARRAY(ARG2, i);  // Get item text
        char *safe_item = sanitize_utf8(item_text);
        char *parent_idx = (i < GETARRAYHI(ARG3)) ? GETSARRAY(ARG3, i) : NULL;

        // Debug print
        printf("Processing item %d: %s (parent: %s)\n",
               i+1, safe_item, parent_idx ? parent_idx : "root");

        GtkTreeIter iter;
        GtkTreeIter *parent_iter = NULL;

        // Look up parent if it exists
        if (parent_idx && *parent_idx) {
            parent_iter = g_hash_table_lookup(iter_table, parent_idx);
            if (!parent_iter) {
                printf("Warning: Parent %s not found for item %s\n", parent_idx, safe_item);
            } else {
                // Get parent's text for verification
                GtkTreeIter parent = *parent_iter;
                gchar *parent_text;
                gtk_tree_model_get(GTK_TREE_MODEL(store), &parent, 0, &parent_text, -1);
                printf("Found parent: %s for item: %s\n", parent_text, safe_item);
                g_free(parent_text);
            }
        }

        // Add node to tree
        gtk_tree_store_append(store, &iter, parent_iter);
        gtk_tree_store_set(store, &iter,
                           0, safe_item,  // Display text
                           1, g_strdup_printf("%d", i+1),  // Store index (1-based)
                           -1);

        // Store iterator for potential children
        char *key = g_strdup_printf("%d", i+1);  // Use 1-based index
        GtkTreeIter *stored_iter = gtk_tree_iter_copy(&iter);
        g_hash_table_insert(iter_table, key, stored_iter);

        g_free(safe_item);
    }

    // Clean up hash table
    g_hash_table_destroy(iter_table);

    // Set selection mode
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    gtk_tree_selection_set_mode(selection, multi_select ? GTK_SELECTION_MULTIPLE : GTK_SELECTION_SINGLE);

    // Add message label if provided
    if (message && *message) {
        char *safe_message = sanitize_utf8(message);
        GtkWidget *label = gtk_label_new(safe_message);
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                           label, FALSE, FALSE, 0);
        g_free(safe_message);
    }

    // Create scrolled window
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scrolled), tree_view);

    // Add scrolled window to dialog
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       scrolled, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    // Run dialog and get result
    char result[32768] = "";
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        GList *selected_rows = gtk_tree_selection_get_selected_rows(selection, NULL);
        for (GList *iter = selected_rows; iter != NULL; iter = iter->next) {
            GtkTreePath *path = (GtkTreePath *)iter->data;
            GtkTreeIter tree_iter;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &tree_iter, path)) {
                gchar *index;
                gtk_tree_model_get(GTK_TREE_MODEL(store), &tree_iter, 1, &index, -1);

            if (result[0]) strcat(result, "|");
                strcat(result, index);
                g_free(index);
        }

            gtk_tree_path_free(path);
    }

        g_list_free(selected_rows);
    }

    // Clean up
    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);
    g_object_unref(store);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX(result);
    ENDPROC
}

PROCEDURE(text_display)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);
    const char *text = GETSTRING(ARG2);  // The diagram string

    // Create a dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title, NULL, GTK_DIALOG_MODAL, "_Close", GTK_RESPONSE_CLOSE, NULL);
    GtkWidget *label = gtk_label_new(text);  // Use GtkLabel for displaying text

    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);  // Enable line wrapping
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), label);

    // Set the default size of the dialog (width, height)
    gtk_window_set_default_size(GTK_WINDOW(dialog), 200, 200);  // Set width to 600 and height to 400

    gtk_widget_show_all(dialog);

    // Run dialog
    gtk_dialog_run(GTK_DIALOG(dialog));

    // Clean up
    gtk_widget_destroy(dialog);
    ENDPROC
}

PROCEDURE(text_display_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);
    int item_count = GETARRAYHI(ARG2);  // Number of items to display

    // Create a temporary top-level window as parent
    GtkWidget *temp_parent = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_object_ref_sink(temp_parent);

    // Create dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
            title,
            GTK_WINDOW(temp_parent),
            GTK_DIALOG_MODAL,
            "_Close", GTK_RESPONSE_CLOSE,
            NULL);

    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    // Create a vertical box for layout
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);

    // Add message label
    if (message && *message) {
        char *safe_message = sanitize_utf8(message);
        GtkWidget *label = gtk_label_new(safe_message);
        gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
        gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
        g_free(safe_message);
    }

    // Create a list box to display items
    GtkWidget *list_box = gtk_list_box_new();

    // Add items to the list box
    for (int i = 0; i < item_count; i++) {
        char *item_text = GETSARRAY(ARG2, i);  // Get item text
        char *safe_item_text = sanitize_utf8(item_text);

        // Create a label for each item
        GtkWidget *item_label = gtk_label_new(safe_item_text);
        gtk_label_set_xalign(GTK_LABEL(item_label), 0.0);  // Set alignment to left
        gtk_list_box_insert(GTK_LIST_BOX(list_box), item_label, -1);

        g_free(safe_item_text);
    }

    // Create a scrolled window to contain the list box
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC,
                                   GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(list_box, -1, 30 * 20);  // Set height for 30 lines (20 pixels each)
    gtk_container_add(GTK_CONTAINER(scrolled_window), list_box);

    // Add the scrolled window to the vertical box
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

    // Add the vertical box to the dialog
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                       vbox, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);

    // Run dialog
    gtk_dialog_run(GTK_DIALOG(dialog));

    // Clean up
    gtk_widget_destroy(dialog);
    g_object_unref(temp_parent);

    while (gtk_events_pending()) {
        gtk_main_iteration_do(FALSE);
    }

    RETURNSTRX("1");  // Indicate dialog was closed
    ENDPROC
}

void print_tree(const char *parent_id, const char *prefix, GString *diagram, GHashTable *children, const char **items) {
    GPtrArray *child_list = g_hash_table_lookup(children, parent_id);
    if (!child_list) return;

    for (guint i = 0; i < child_list->len; i++) {
        const char *child_id = g_ptr_array_index(child_list, i);
        int idx = atoi(child_id) - 1;
        gboolean is_last = (i == child_list->len - 1);

        // Add current item
        g_string_append_printf(diagram, "%s%s %s\n",
                               prefix,
                               is_last ? "└──" : "├──",
                               items[idx]);

        // Recurse for children
        char *new_prefix = g_strdup_printf("%s%s ",
                                           prefix,
                                           is_last ? "   " : "│  ");
        print_tree(child_id, new_prefix, diagram, children, items);
        g_free(new_prefix);
    }
}

PROCEDURE(tree_diagram)
{
    int item_count = GETARRAYHI(ARG0);  // items[]

    // Create arrays to hold items and parents
    const char **items = malloc(item_count * sizeof(char*));
    const char **parents = malloc(item_count * sizeof(char*));

    // Copy items and parents from REXX arrays
    for (int i = 0; i < item_count; i++) {
        items[i] = GETSARRAY(ARG0, i);  // Get item text
        parents[i] = (i < GETARRAYHI(ARG1)) ? GETSARRAY(ARG1, i) : "";
    }

    // Create the diagram
    GString *diagram = g_string_new("");
    GHashTable *children = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
                                                 (GDestroyNotify)g_ptr_array_unref);

    // First pass: Group children by parent
    for (int i = 0; i < item_count; i++) {
        const char *parent_idx = parents[i];
        char *key = parent_idx && *parent_idx ? g_strdup(parent_idx) : g_strdup("");

        GPtrArray *child_list = g_hash_table_lookup(children, key);
        if (!child_list) {
            child_list = g_ptr_array_new_with_free_func(g_free);
            g_hash_table_insert(children, g_strdup(key), child_list);
        }
        g_ptr_array_add(child_list, g_strdup_printf("%d", i + 1));
        g_free(key);
    }

    // Start with root item
    if (item_count > 0) {
        g_string_append_printf(diagram, "%s\n", items[0]);
        print_tree("", "", diagram, children, items);
    }

    // Clean up
    g_hash_table_destroy(children);
    free(items);
    free(parents);

    // Return the diagram
    char *result = g_strdup(diagram->str);
    g_string_free(diagram, TRUE);
    RETURNSTRX(result);
    g_free(result);
    ENDPROC
}

PROCEDURE(splash_pick)
{
    static int gtk_initialized = 0;

    if (!gtk_initialized) {
        if (!gtk_init_check(NULL, NULL)) {
            RETURNSTRX("");
        }
        gtk_initialized = 1;
    }

    const char *title = GETSTRING(ARG0);
    const char *message = GETSTRING(ARG1);
    int duration = GETINT(ARG2);
    int width    = GETINT(ARG3);
    int height   = GETINT(ARG4);
    const char *image_path = GETSTRING(ARG5);

    // Create a window for the splash screen
    GtkWidget *splash_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(splash_window), title);
    gtk_window_set_decorated(GTK_WINDOW(splash_window), FALSE);
    gtk_window_set_position(GTK_WINDOW(splash_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(splash_window), width, height);

    // Create an overlay to stack widgets
    GtkWidget *overlay = gtk_overlay_new();
    gtk_container_add(GTK_CONTAINER(splash_window), overlay);

    // Create and scale background image
    GdkPixbuf *original_pixbuf = gdk_pixbuf_new_from_file(image_path, NULL);
    if (original_pixbuf) {
        // Scale the image to window size (500x500)
        GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(
                original_pixbuf,
                width,  // width
                height,  // height
                GDK_INTERP_BILINEAR
        );
        g_object_unref(original_pixbuf);

        GtkWidget *background = gtk_image_new_from_pixbuf(scaled_pixbuf);
        gtk_container_add(GTK_CONTAINER(overlay), background);
        g_object_unref(scaled_pixbuf);
    } else {
        GtkWidget *background = gtk_image_new();
        gtk_container_add(GTK_CONTAINER(overlay), background);
    }
    // Create CSS provider specifically for splash screen
    GtkCssProvider *splash_provider = gtk_css_provider_new();
    if(strlen(image_path)==0) {
        gtk_css_provider_load_from_data(splash_provider,
                                        "label { color: blue; }"
                                        ".title { font-size: 24px; font-weight: bold; }"
                                        ".message { font-size: 20px; font-weight: normal; }",
                                        -1, NULL);
    } else {
        gtk_css_provider_load_from_data(splash_provider,
                                        "label { color: white; }"
                                        ".title { font-size: 24px; font-weight: bold; }"
                                        ".message { font-size: 20px; font-weight: normal; }",
                                        -1, NULL);

    }
    GtkStyleContext *context;

    // Create main vertical box
    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), main_vbox);

    // Add title at the top
    GtkWidget *title_label = gtk_label_new(title);
    context = gtk_widget_get_style_context(title_label);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(splash_provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_class(context, "title");
    gtk_widget_set_margin_top(title_label, 20);
    gtk_box_pack_start(GTK_BOX(main_vbox), title_label, FALSE, FALSE, 0);

    // Create bottom box for message and progress bar
    GtkWidget *bottom_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(bottom_vbox, 20);
    gtk_widget_set_margin_end(bottom_vbox, 20);
    gtk_widget_set_margin_bottom(bottom_vbox, 20);
    gtk_box_pack_end(GTK_BOX(main_vbox), bottom_vbox, FALSE, FALSE, 0);

    // Add message label with larger text
    GtkWidget *label = gtk_label_new(message);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    context = gtk_widget_get_style_context(label);
    gtk_style_context_add_provider(context,
                                   GTK_STYLE_PROVIDER(splash_provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_style_context_add_class(context, "message");  // Add message class
    gtk_box_pack_start(GTK_BOX(bottom_vbox), label, FALSE, FALSE, 0);

    // Add progress bar
    GtkWidget *progress = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);
    gtk_box_pack_start(GTK_BOX(bottom_vbox), progress, FALSE, FALSE, 0);

    g_object_unref(splash_provider);

    gtk_widget_show_all(splash_window);

    // Update progress bar in steps
    int steps = 20;
    int step_duration = (duration * 1000000) / steps;

    for (int i = 0; i <= steps; i++) {
        gdouble fraction = (gdouble)i / steps;
        char progress_text[32];
        g_snprintf(progress_text, sizeof(progress_text), "%d%%", (int)(fraction * 100));

        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), fraction);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), progress_text);

        while (gtk_events_pending())
            gtk_main_iteration();

        g_usleep(step_duration);
    }

    // Destroy the splash window
    gtk_widget_destroy(splash_window);

    // Process any remaining events
    while (gtk_events_pending())
        gtk_main_iteration();

    RETURNSTRX("");  // Add explicit return
    ENDPROC
}

PROCEDURE(add_checkbox)
{
    if (widget_count >= MAX_WIDGETS) {
        RETURNINTX(-1);  // Handle error - too many checkboxes
    }

    const char *checkbox_text = GETSTRING(ARG0);
    int x = GETINT(ARG1);
    int y = GETINT(ARG2);

    // Create the checkbox
    GtkWidget *checkbox = gtk_check_button_new_with_label(checkbox_text);
    gtk_fixed_put(GTK_FIXED(main_fixed), checkbox, x, y);
    gtk_widget_show(checkbox);

    // Store the widget address
    if (widget_count < MAX_WIDGETS) {
        widgets[widget_count++] = checkbox;  // Add checkbox to the widgets array
    }

    RETURNINT(widget_count);  // Return the index+1 (to have number >0) of the widget in the widgets array
ENDPROC
}

PROCEDURE(hide_widget) {
    int index = GETINT(ARG0)-1;   // Convert to zero-based index

    if (index < 0 || index >= widget_count) {
        RETURNINTX(-1);  // Invalid index
    }

    // Hide the specified widget
    gtk_widget_hide(widgets[index]);
    RETURNINT(1);  // Success
ENDPROC
}

PROCEDURE(report_widgets) {
    char report[2048];  // Buffer to hold the report (increased size for text)
    int length = 0;     // Length of the report

    // Iterate through the widgets array
    for (int i = 0; i < widget_count; i++) {
        if (widgets[i]) {
            const char *widget_type = NULL;
            const char *widget_text = NULL;

            // Debugging output
            // g_print("Processing widget %d of type: %s\n", i + 1, g_type_name(G_OBJECT_TYPE(widgets[i])));

            // Determine the type of the widget
            if (GTK_IS_BUTTON(widgets[i])) {
                widget_text = gtk_button_get_label(GTK_BUTTON(widgets[i]));
            } else if (GTK_IS_LABEL(widgets[i])) {
                widget_text = gtk_label_get_text(GTK_LABEL(widgets[i]));
            } else if (GTK_IS_CHECK_BUTTON(widgets[i])) {
                widget_text = gtk_button_get_label(GTK_BUTTON(widgets[i]));  // Get label of checkbox
            } else if (GTK_IS_COMBO_BOX(widgets[i])) {
                widget_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widgets[i]));
            } else if (GTK_IS_ENTRY(widgets[i])) {
                widget_text = gtk_entry_get_text(GTK_ENTRY(widgets[i]));
            } else {
                widget_type = "Unknown";
            }
            widget_type=g_type_name(G_OBJECT_TYPE(widgets[i]));
            // Append widget information to the report
            length += snprintf(report + length, sizeof(report) - length, 
                               "Widget %d: %s", i + 1, widget_type+3);
            if (widget_text) {
                length += snprintf(report + length, sizeof(report) - length, 
                                   " (Text: %s)\n", widget_text);
            } else {
                length += snprintf(report + length, sizeof(report) - length, 
                                   " (Text: None)\n");
            }
        }
    }

    // Return the report as a string
    RETURNSTRX(report);
    ENDPROC
}

PROCEDURE(set_edit) {
    int index = GETINT(ARG0) - 1;  // Convert to zero-based index
    const char *text = GETSTRING(ARG1);
    
    // Check if the index is valid
    if (index < 0 || index >= widget_count) {
        RETURNINT(-1);  // Invalid index
    }
    
    // Check if the widget is an entry
    if (widgets[index] && GTK_IS_ENTRY(widgets[index])) {
        gtk_entry_set_text(GTK_ENTRY(widgets[index]), text);  // Set the text of the entry
        RETURNINT(1);  // Success
    }
    
    RETURNINT(-1);  // Not found or not an entry
    ENDPROC
}


PROCEDURE(show_widget) {
    int index = GETINT(ARG0) - 1;  // Convert to zero-based index

    if (index < 0 || index >= widget_count) {
        RETURNINTX(-1);  // Invalid index
    }

    // Show the specified widget
    gtk_widget_show(widgets[index]);
    RETURNINT(1);  // Success
ENDPROC
}

PROCEDURE(run_external_program) {
    const char *command = GETSTRING(ARG0);  // Get the command to run
    const char *argv = GETSTRING(ARG1);     // Get the parms to use
    const char *wlib = GETSTRING(ARG2);    // Get the command to run

    GError *error = NULL;
    gboolean success;

    // Run the external program asynchronously
    success = g_spawn_async(NULL,  // Use the default working directory
                            (char *[]) {(char *) command, NULL},  // Command to execute
                            NULL,  // No environment variables
                            G_SPAWN_SEARCH_PATH,  // Search for the command in the PATH
                            NULL,  // No child setup function
                            NULL,  // No user data for child setup
                            NULL,  // No child process ID
                            &error);  // Capture any error

    if (!success) {
        // Handle error
        g_print("Error running command: %s\n", error->message);
        g_error_free(error);  // Free the error
        RETURNINTX(-8);  // Indicate failure
    }

    RETURNINT(1);  // Indicate success
    ENDPROC
}

PROCEDURE(run_external_program_sync) {
    const char *command = GETSTRING(ARG0);  // Get the command to run
    const char *args = GETSTRING(ARG1);  // Get additional arguments as a comma-separated string
    const char *working_directory = GETSTRING(ARG2);  // Get the working directory

    GError *error = NULL;
    gint exit_status;
    gchar *stdout_data = NULL;
    gchar *stderr_data = NULL;
    char *resulterror = NULL;  // Moved declaration to top
    char *resultx = NULL;      // Moved declaration to top

    /* Debug output for working directory
    g_print("Working Directory: %s\n", working_directory ? working_directory : "(null)");
    g_print("Command: %s\n", command ? command : "(null)");
    g_print("Arguments: %s\n", args ? args : "(null)");
    */

    // Split the args string into an array
    gchar **argv = g_strsplit(args, " ", -1);  // Split by space instead of comma

    // Add the command to the argument array
    gchar **full_argv = g_new(gchar *, g_strv_length(argv) + 2);
    full_argv[0] = (gchar *) command;  // First argument is the command

    for (int i = 0; argv[i] != NULL; i++) {
        full_argv[i + 1] = argv[i];  // Copy the additional arguments
    }
    full_argv[g_strv_length(argv) + 1] = NULL;  // Null-terminate the array

    // Run the external program synchronously
    gboolean success = g_spawn_sync(working_directory,  // Working directory
                                    full_argv,  // Command and arguments
                                    NULL,  // Environment
                                    G_SPAWN_SEARCH_PATH,  // Flags
                                    NULL,  // Child setup function
                                    NULL,  // User data
                                    &stdout_data,  // Standard output
                                    &stderr_data,  // Standard error
                                    &exit_status,  // Exit status
                                    &error);  // Error

    // Free the argument arrays
    g_strfreev(argv);
    g_free(full_argv);

    if (!success) {
        // Handle error
        g_print("Error running command: %s\n", error->message);
        g_error_free(error);  // Free the error
        if (stderr_data) {
            resultx = g_strdup_printf("Exit-Code %d\n%s", -8, stderr_data ? stderr_data : "");
            g_free(stdout_data);
            g_free(stderr_data);
            RETURNSTRX(resulterror);
        }
        RETURNSTRX("-8");
    }

    // Print the error output if available, preserving line breaks
    if (stderr_data && *stderr_data) {
        resultx = g_strdup_printf("Exit-Code %d\n%s", exit_status, stderr_data ? stderr_data : "");
    }

    resultx = g_strdup_printf("Exit-Code %d\n%s", exit_status, stdout_data ? stdout_data : "");
    g_free(stdout_data);
    g_free(stderr_data);
    RETURNSTR(resultx);
    ENDPROC
}

PROCEDURE(set_sensitive) {
    int widget_index = GETINT(ARG0) - 1;  // Convert to zero-based index
    int sensitive = GETINT(ARG1);         // 1 for sensitive (enabled), 0 for insensitive (disabled)

    // Check if the index is valid
    if (widget_index < 0 || widget_index >= widget_count) {
        RETURNINT(-1);  // Invalid index
    }

    // Set the widget's sensitivity
    gtk_widget_set_sensitive(widgets[widget_index], sensitive);

    RETURNINT(0);  // Success
    ENDPROC
}

PROCEDURE(copy_file_procedure) {
    const char *source_path = GETSTRING(ARG0);  // Source file path
    const char *destination_path = GETSTRING(ARG1);  // Destination file path

    GError *error = NULL;

    // Call the copy function
    if (copy_file(source_path, destination_path, &error)) {
        RETURNINT(0);  // Success
    } else {
        g_print("Error copying file: %s\n", error->message);
        g_error_free(error);  // Free the error
        RETURNINT(-8);  // Failure
    }
}

gboolean copy_file(const char *source_path, const char *destination_path, GError **error) {
    GFile *source_file = g_file_new_for_path(source_path);
    GFile *destination_file = g_file_new_for_path(destination_path);

    // Copy the file
    return g_file_copy(source_file, destination_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, error);
}
/* Add this with the other function prototypes at the top */
static gboolean draw_graph(GtkWidget *widget, cairo_t *cr, gpointer data);

#define drawxtick(cr,xpos,yzero,tlen) {cairo_move_to(cr, xpos, yzero - tlen);  \
                                       cairo_line_to(cr, xpos, yzero + tlen); \
                                       cairo_stroke(cr);}  // Ensure to stroke the line to make it visible
#define drawytick(cr,xzero,ypos,tlen) {cairo_move_to(cr, xzero-tlen, ypos);   \
                                       cairo_line_to(cr, xzero+tlen, ypos);   \
                                       cairo_stroke(cr);}  // Ensure to stroke the line to make it visible


#define drawxnumber(cr,xpos,yzero,num) {char number[10]; snprintf(number, sizeof(num), "%g", num);  \
                                      cairo_move_to(cr, xpos - 8, yzero + 15);  \
                                      cairo_show_text(cr, number);}

#define drawynumber(cr,xzero,ypos,num) {char number[10]; snprintf(number, sizeof(num), "%g", num);  \
                                      cairo_move_to(cr, xzero +10, ypos + 4);  \
                                      cairo_show_text(cr, number);}
#define drawdot(cr,px,py,size)  cairo_new_path(cr); \
                                cairo_arc(cr, px, py, size,0, 2 * G_PI);  \
                                cairo_close_path(cr); \
                                cairo_fill(cr);

// Function to find the "nice" step size
double find_tick_step(double min_val, double max_val, int num_ticks) {
    double range = max_val - min_val;
    double raw_step = range / num_ticks; // Initial step estimate

    // Get the base power of 10 (10^x) closest to raw_step
    double power = pow(10, floor(log10(raw_step)));

    // Adjust to nearest 1, 2, or 5 times the power of 10
    if (raw_step / power >= 5) {
        return 5 * power;
    } else if (raw_step / power >= 2) {
        return 2 * power;
    } else {
        return power;
    }
}

typedef struct GraphDetails_w {
    GtkWidget *drawing_area;
    cairo_t *graph;
    double  width;
    double  height;
    double  x_zero_pos;
    double  y_zero_pos;
    double  x_range;
    double  y_range;
    double  scale_x;
    double  scale_y;
    double  step_x;
    double  step_y;
} graphs;

/* Add this with the other procedures */

graphs graphArray[64];

int granges(void *xarray,void *yarray, int width,int height) {

    int xhi = GETARRAYHI(xarray);
    if (xhi <= 0) return FALSE; // No data to plot

    double x_min = GETFARRAY(xarray, 0);
    double x_max = x_min;
    double y_min = GETFARRAY(yarray, 0);
    double y_max = y_min;

// Step 1: Find actual min/max values for x and y
    for (int i = 1; i < xhi; i++) {
        double x = GETFARRAY(xarray, i);
        double y = GETFARRAY(yarray, i);
        x_min = fmin(x_min, x);
        x_max = fmax(x_max, x);
        y_min = fmin(y_min, y);
        y_max = fmax(y_max, y);
    }

// Adjust axis limits for padding
    x_min = (x_min > 0) ? 0 : x_min - 0.5;
    x_max += 0.5;
    y_min = (y_min > 0) ? 0 : y_min - 0.5;
    y_max += 0.5;

    int num_ticks = 5;   // ticks without numbers between numbered ticks
    int steps = 5;
    double step_x = find_tick_step(x_min, x_max, num_ticks) / steps;
    double step_y = find_tick_step(y_min, y_max, num_ticks) / steps;

// Calculate ranges and scales
    double x_range = fmax(fabs(x_max), fabs(x_min));
    double y_range = fmax(fabs(y_max), fabs(y_min));

    double scale_x = (width - 15) / (x_range * 2);
    double scale_y = (height - 15) / (y_range * 2);

// Set zero positions based on the center
    double x_zero_pos = width / 2;
    double y_zero_pos = height / 2;
    graphArray[0].graph = 0;
    graphArray[0].scale_x = scale_x;
    graphArray[0].scale_y = scale_y;
    graphArray[0].y_zero_pos = y_zero_pos;
    graphArray[0].x_zero_pos = x_zero_pos;
    graphArray[0].width = width;
    graphArray[0].height = height;
    graphArray[0].x_range = x_range;
    graphArray[0].y_range = y_range;
    graphArray[0].step_x = step_x;
    graphArray[0].step_y = step_y;

}


// Function to add a new graph to the existing graph widget

PROCEDURE(add_graph) {
    int x = GETINT(ARG0);
    int y = GETINT(ARG1);
    int width = GETINT(ARG2);
    int height = GETINT(ARG3);
    int function_type = GETINT(ARG4);

    granges(ARG5,ARG6, width, height);
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, width, height);
    
    // Store array arguments in widget data
    g_object_set_data(G_OBJECT(drawing_area), "xarray", ARG5);
    g_object_set_data(G_OBJECT(drawing_area), "yarray", ARG6);
    
    g_signal_connect(G_OBJECT(drawing_area), "draw",
                    G_CALLBACK(draw_graph), GINT_TO_POINTER(function_type));
    
    gtk_fixed_put(GTK_FIXED(main_fixed), drawing_area, x, y);
    gtk_widget_show(drawing_area);

    widgets[widget_count] = drawing_area;
    graphArray[0].drawing_area = drawing_area;
 //   graphArray[widget_count]=graphArray[0];    // not yet set!

    printf("drawing %d %d\n",widget_count, drawing_area);
    widget_count++;
    RETURNINT(widget_count);
}

#define xcheck(var) printf(#var" %g=%g\n",var,graphArray[0].var);

gboolean draw_graph(GtkWidget *widget, cairo_t *cr, gpointer data) {
 //   int width = gtk_widget_get_allocated_width(widget);
 //   int height = gtk_widget_get_allocated_height(widget);
    int function_type = GPOINTER_TO_INT(data);

    void *xarray = g_object_get_data(G_OBJECT(widget), "xarray");
    void *yarray = g_object_get_data(G_OBJECT(widget), "yarray");

    int xhi = GETARRAYHI(xarray);
    if (xhi <= 0) return FALSE; // No data to plot
/*
    double x_min = GETFARRAY(xarray, 0);
    double x_max = x_min;
    double y_min = GETFARRAY(yarray, 0);
    double y_max = y_min;

    // Step 1: Find actual min/max values for x and y
    for (int i = 1; i < xhi; i++) {
        double x = GETFARRAY(xarray, i);
        double y = GETFARRAY(yarray, i);
        x_min = fmin(x_min, x);
        x_max = fmax(x_max, x);
        y_min = fmin(y_min, y);
        y_max = fmax(y_max, y);
    }

    // Adjust axis limits for padding
    x_min = (x_min > 0) ? 0 : x_min - 0.5;
    x_max += 0.5;
    y_min = (y_min > 0) ? 0 : y_min - 0.5;
    y_max += 0.5;

    int num_ticks = 5;   // ticks without numbers between numbered ticks
    int steps = 5;
    double step_x = find_tick_step(x_min, x_max, num_ticks) / steps;
    double step_y = find_tick_step(y_min, y_max, num_ticks) / steps;

    // Calculate ranges and scales
    double x_range = fmax(fabs(x_max), fabs(x_min));
    double y_range = fmax(fabs(y_max), fabs(y_min));

    double scale_x = (width - 15) / (x_range * 2);
    double scale_y = (height - 15) / (y_range * 2);

    // Set zero positions based on the center
    double x_zero_pos = width / 2;
    double y_zero_pos = height / 2;
*/
    double scale_x=graphArray[0].scale_x;
    double scale_y=graphArray[0].scale_y;
    double y_zero_pos=graphArray[0].y_zero_pos;
    double x_zero_pos=graphArray[0].x_zero_pos;
    int width=graphArray[0].width;
    int height=graphArray[0].height;
    double  x_range=graphArray[0].x_range;
    double y_range=graphArray[0].y_range;
    double step_x=graphArray[0].step_x;
    double  step_y=graphArray[0].step_y;

     // Draw axes
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 0, 0, 0);

    // X-axis
    cairo_move_to(cr, 0, y_zero_pos);
    cairo_line_to(cr, width, y_zero_pos);
    cairo_stroke(cr);

    // Y-axis
    cairo_move_to(cr, x_zero_pos, 0);
    cairo_line_to(cr, x_zero_pos, height);
    cairo_stroke(cr);

    // Draw X-axis markers
    int j = 0;
    for (double i = -x_range; i <= x_range + step_x; i += step_x) {
        double x_pos = x_zero_pos + (i * scale_x);
        if (x_pos > width) break;
        if (x_pos >= 0) {
            if (j % 5 == 0) {
                drawxtick(cr, x_pos, y_zero_pos, 5); // Major tick
                drawxnumber(cr, x_pos, y_zero_pos, i);
            } else {
                drawxtick(cr, x_pos, y_zero_pos, 2); // Minor tick
            }
            j++;
        }
    }

    // Draw Y-axis markers
    j = 0;
    for (double i = -y_range; i <= y_range + step_y; i += step_y) {
        double y_pos = y_zero_pos - (i * scale_y);
        if (y_pos > height) break;
        if (y_pos <= 0) continue;
        if (j % 5 == 0) {
            drawytick(cr, x_zero_pos, y_pos, 5); // Major tick
            drawynumber(cr, x_zero_pos, y_pos, i);
        } else {
            drawytick(cr, x_zero_pos, y_pos, 2); // Minor tick
        }
        j++;
    }

    // Draw the actual graph
    cairo_set_source_rgb(cr, 0, 0, 1);

    if (function_type == 2) { // Points-only mode
        for (int i = 0; i < xhi; i++) {
            double x = GETFARRAY(xarray, i);
            double y = GETFARRAY(yarray, i);
            double px = x_zero_pos + (x * scale_x);
            double py = height / 2 - (y * scale_y);
            drawdot(cr, px, py, 1);
            printf("add %g %g\n",px,py);
        }
    } else { // Connected line mode
        double x0 = GETFARRAY(xarray, 0);
        double y0 = GETFARRAY(yarray, 0);
        double px0 = x_zero_pos + (x0 * scale_x);
        double py0 = height / 2 - (y0 * scale_y);
        cairo_move_to(cr, px0, py0);

        for (int i = 1; i < xhi; i++) {
            double x = GETFARRAY(xarray, i);
            double y = GETFARRAY(yarray, i);
            cairo_line_to(cr, x_zero_pos + (x * scale_x), height / 2 - (y * scale_y));
        }
        cairo_stroke(cr);
    }

    // Draw origin tick marks
    cairo_move_to(cr, x_zero_pos, y_zero_pos - 5);
    cairo_line_to(cr, x_zero_pos, y_zero_pos + 5);
    cairo_stroke(cr);

    cairo_move_to(cr, x_zero_pos - 5, y_zero_pos);
    cairo_line_to(cr, x_zero_pos + 5, graphArray[0].y_zero_pos);
    cairo_stroke(cr);

    return FALSE;
}


LOADFUNCS
    ADDPROC(init_window, "gui.init_window", "b", ".int", "title=.string, width=.int,height=.int");
    ADDPROC(add_button, "gui.add_button", "b", ".int", "button_text=.string,x=.int,y=.int");
    ADDPROC(add_text, "gui.add_text", "b", ".int", "text=.string,x=.int,y=.int");
    ADDPROC(show_window, "gui.show_window", "b", ".int", "x=.int,y=.int");
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
    ADDPROC(list_get_selected_item, "gui.list_get_selected_item", "b", ".string", "list=.int");
    ADDPROC(list_set_header, "gui.list_header", "b", ".int", "list=.int,header=.string,text_color=.string,bg_color=.string");
    ADDPROC(add_edit, "gui.add_edit", "b", ".int", "x=.int,y=.int,width=.int");
    ADDPROC(get_widget_address, "gui.get_widget_address", "b", ".int", "index=.int");
    ADDPROC(rgb_to_hex, "gui.rgb_to_hex", "b", ".string", "r=.int,g=.int,b=.int");
    ADDPROC(set_text, "gui.set_text", "b", ".int", "index=.int,text=.string");
    ADDPROC(add_message_area, "gui.add_message_area", "b", ".int", "x=.int,y=.int,width=.int,height=.int");
    ADDPROC(append_message, "gui.append_message", "b", ".int", "index=.int,message=.string");
    ADDPROC(add_status_bar, "gui.add_status_bar", "b", ".int", "");
    ADDPROC(set_status, "gui.set_status", "b", ".int", "index=.int,message=.string");
    ADDPROC(list_get_count, "gui.list_get_count", "b", ".int", "list=.int");
    ADDPROC(file_pick, "gui.file_pick", "b", ".string", "title=.string,initial_dir=.string,save_dialog=.int,pattern=.string");
    ADDPROC(path_pick, "gui.path_pick", "b", ".string", "title=.string,initial_dir=.string");
    ADDPROC(date_pick, "gui.date_pick", "b", ".string", "title=.string,show_time=.int,format=.string");
    ADDPROC(list_pick, "gui.list_pick", "b", ".string", "title=.string,expose items=.string[],multi_select=.int,message=.string");
    ADDPROC(dialog_pick, "gui.dialog_pick", "b", ".string", "title=.string,message=.string,buttons=.string");
    ADDPROC(input_pick, "gui.input_pick", "b", ".string", "title=.string,message=.string,default_value=.string,password_mode=.int");
    ADDPROC(form_pick, "gui.form_pick", "b", ".string", "title=.string,message=.string,expose labels=.string[],expose defaults=.string[]");
    ADDPROC(notify_pick, "gui.notify_pick", "b", ".string", "title=.string,message=.string,type=.string");
    ADDPROC(combo_pick, "gui.combo_pick", "b", ".string", "title=.string,message=.string,expose items=.string[]");
    ADDPROC(page_pick, "gui.page_pick", "b", ".string", "title=.string,expose pages=.string[],expose labels=.string[],expose defaults=.string[]");
    ADDPROC(tree_pick, "gui.tree_pick", "b", ".string", "title=.string,message=.string,expose items=.string[],expose parents=.string[],multi_select=.int");
    ADDPROC(text_display_pick, "gui.text_display_pick", "b", ".string", "title=.string,message=.string,expose item_texts=.string[]");
    ADDPROC(text_display, "gui.text_display", "b", ".string", "title=.string,message=.string,item_texts=.string");
    ADDPROC(tree_diagram, "gui.tree_diagram", "b", ".string", "expose items=.string[],expose parents=.string[]");
    ADDPROC(splash_pick, "gui.splash_pick", "b", ".string", "title=.string,message=.string,duration=.int,width=.int,height=.int,image_path=.string");
    ADDPROC(add_checkbox, "gui.add_checkbox", "b", ".int", "text=.string,x=.int,y=.int");
    ADDPROC(hide_widget, "gui.hide_widget", "b", ".int", "index=.int");
    ADDPROC(report_widgets, "gui.report_widgets", "b", ".string", "");
    ADDPROC(set_edit, "gui.set_edit", "b", ".int", "index=.int,text=.string");
    ADDPROC(show_widget, "gui.show_widget", "b", ".int", "index=.int");
    ADDPROC(run_external_program, "gui.run_program", "b", ".string", "command=.string");
    ADDPROC(run_external_program_sync, "gui.run_sync", "b", ".string", "command=.string,parm=.string,wdir=.string");
    ADDPROC(set_sensitive, "gui.set_sensitive", "b", ".int", "index=.int,sensitive=.int");
    ADDPROC(copy_file_procedure, "gui.copy_file", "b", ".int", "source_path=.string,destination_path=.string");
    ADDPROC(add_graph, "gui.add_graph", "b", ".int", "x=.int,y=.int,width=.int,height=.int,function_type=.int,x_values=.float[],y_values=.float[]");
 ENDLOADFUNCS
