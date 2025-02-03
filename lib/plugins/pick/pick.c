//
// System Information Plugin for crexx/pa - Plugin Architecture
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>   // For POSIX systems (Linux/macOS)
#include "crexxpa.h"    // crexx/pa - Plugin Architecture header file
#include <gtk/gtk.h>

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

LOADFUNCS
    ADDPROC(file_pick, "pick.file_pick", "b", ".string", "title=.string,initial_dir=.string,save_dialog=.int");
    ADDPROC(path_pick, "pick.path_pick", "b", ".string", "title=.string,initial_dir=.string");
    ADDPROC(date_pick, "pick.date_pick", "b", ".string", "title=.string,show_time=.int,format=.string");
    ADDPROC(list_pick, "pick.list_pick", "b", ".string", "title=.string,expose items=.string[],multi_select=.int,message=.string");
    ADDPROC(dialog_pick, "pick.dialog_pick", "b", ".string", "title=.string,message=.string,buttons=.string");
    ADDPROC(input_pick, "pick.input_pick", "b", ".string", "title=.string,message=.string,default_value=.string,password_mode=.int");
    ADDPROC(form_pick, "pick.form_pick", "b", ".string", "title=.string,message=.string,expose labels=.string[],expose defaults=.string[]");
    ADDPROC(notify_pick, "pick.notify_pick", "b", ".string", "title=.string,message=.string,type=.string");
    ADDPROC(combo_pick, "pick.combo_pick", "b", ".string", "title=.string,message=.string,expose items=.string[]");
    ADDPROC(page_pick, "pick.page_pick", "b", ".string", "title=.string,expose pages=.string[],expose labels=.string[],expose defaults=.string[]");
    ADDPROC(tree_pick, "pick.tree_pick", "b", ".string", "title=.string,message=.string,expose items=.string[],expose parents=.string[],multi_select=.int");
    ADDPROC(text_display_pick, "pick.text_display_pick", "b", ".string", "title=.string,message=.string,expose item_texts=.string[]");
    ADDPROC(text_display, "pick.text_display", "b", ".string", "title=.string,message=.string,item_texts=.string");
    ADDPROC(tree_diagram, "pick.tree_diagram", "b", ".string", "expose items=.string[],expose parents=.string[]");
ENDLOADFUNCS;