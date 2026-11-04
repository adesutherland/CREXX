/* Minimal GTK 3 driver for the experimental cREXX UI library. */
#include <gtk/gtk.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "crexxpa.h"

#define UI_GTK_MAX_NODES 64

/* Publish the stable provider identity for compiler/native packaging. GTK and
 * the process-owned window state remain on the conservative legacy lane; this
 * does not claim process reentrancy or VM-session affinity. */
static uint32_t ui_gtk_procedure_capabilities(const char *name) {
    (void)name;
    return 0u;
}
RXPA_PLUGIN_PROCEDURE_CAPABILITIES(ui_gtk_procedure_capabilities)

typedef struct ui_gtk_node ui_gtk_node;
typedef struct ui_gtk_option {
    char *id;
    GtkWidget *widget;
    ui_gtk_node *owner;
} ui_gtk_option;

struct ui_gtk_node {
    char *id;
    char *action;
    GtkWidget *widget;
    guint status_context;
    ui_gtk_option *options;
    int option_count;
    gboolean owns_widget;
};

typedef struct ui_gtk_loop {
    rxpa_attribute_value handler;
    rxpa_attribute_value scratch_and_result;
    rxpa_attribute_value signal;
    int event_count;
    gboolean callback_failed;
} ui_gtk_loop;

static GtkWidget *ui_window;
static GtkWidget *ui_root;
static GtkWidget *ui_grid;
static ui_gtk_node ui_nodes[UI_GTK_MAX_NODES];
static int ui_node_count;
static int ui_rendering;
static ui_gtk_loop *ui_active_loop;
static const char ui_handler_descriptor[] =
        "rxsig1|on_native_event|.int|action=.string,payload=.string";


/* Native signals capture owned facts. Only one idle source crosses RXPA, so
 * gtk_widget_destroy()/dialog responses cannot re-enter the active cREXX turn.
 * No borrowed RXPA value survives the enclosing synchronous run(). */
typedef struct ui_native_event {
    char *action;
    char *payload;
} ui_native_event;
static GQueue ui_events = G_QUEUE_INIT;
static guint ui_idle_source;
static GtkWidget *ui_dialog;
static gboolean ui_dialog_is_file;
static int ui_more_work;
static gboolean ui_gtk_idle(gpointer ignored);

static void ui_gtk_discard_events(void) {
    ui_native_event *event;
    while ((event = g_queue_pop_head(&ui_events)) != NULL) {
        g_free(event->action);
        g_free(event->payload);
        g_free(event);
    }
}

static void ui_gtk_enqueue(const char *action, const char *payload) {
    ui_native_event *event;
    if (!ui_active_loop) return;
    if (g_queue_get_length(&ui_events) >= 128 || strlen(payload ? payload : "") > 16384) {
        ui_active_loop->callback_failed = TRUE;
        if (gtk_main_level() > 0) gtk_main_quit();
        return;
    }
    event = g_new0(ui_native_event, 1);
    event->action = g_strdup(action);
    event->payload = g_strdup(payload ? payload : "");
    g_queue_push_tail(&ui_events, event);
    if (!ui_idle_source) ui_idle_source = g_idle_add(ui_gtk_idle, NULL);
}

static void ui_gtk_clear_nodes(void) {
    int index;
    for (index = 0; index < UI_GTK_MAX_NODES; ++index) {
        int option;
        for (option = 0; option < ui_nodes[index].option_count; ++option) {
            ui_gtk_option *item = &ui_nodes[index].options[option];
            if (item->widget) {
                g_signal_handlers_disconnect_by_data(item->widget, item);
                g_object_unref(item->widget);
            }
            g_free(item->id);
        }
        g_free(ui_nodes[index].options);
        if (ui_nodes[index].owns_widget && ui_nodes[index].widget) {
            g_signal_handlers_disconnect_by_data(ui_nodes[index].widget, &ui_nodes[index]);
            g_object_unref(ui_nodes[index].widget);
        }
        g_free(ui_nodes[index].id);
        g_free(ui_nodes[index].action);
        ui_nodes[index].id = NULL;
        ui_nodes[index].action = NULL;
        ui_nodes[index].widget = NULL;
        ui_nodes[index].options = NULL;
        ui_nodes[index].option_count = 0;
        ui_nodes[index].owns_widget = FALSE;
    }
    ui_node_count = 0;
}

static gboolean ui_gtk_dispatch(const char *action, const char *payload) {
    rxpa_attribute_value args[2];
    int rc;

    if (!ui_active_loop) return TRUE;

    SETNUMATTRS(ui_active_loop->scratch_and_result, 2);
    args[0] = GETATTR(ui_active_loop->scratch_and_result, 0);
    args[1] = GETATTR(ui_active_loop->scratch_and_result, 1);
    SETSTRING(args[0], action ? action : "");
    SETSTRING(args[1], payload ? payload : "");

    rc = CALLMETHODX(ui_active_loop->handler,
                     ui_handler_descriptor,
                     2,
                     args,
                     ui_active_loop->scratch_and_result,
                     ui_active_loop->signal);
    if (rc != 0) {
        ui_active_loop->callback_failed = TRUE;
        if (gtk_main_level() > 0) gtk_main_quit();
        return FALSE;
    }

    ui_active_loop->event_count++;
    ui_more_work = GETINT(ui_active_loop->scratch_and_result) == 2;
    if (GETINT(ui_active_loop->scratch_and_result) == 0) {
        if (gtk_main_level() > 0) gtk_main_quit();
        return FALSE;
    }
    return TRUE;
}


static gboolean ui_gtk_idle(gpointer ignored) {
    ui_native_event *event = g_queue_pop_head(&ui_events);
    gboolean keep;
    (void)ignored;
    keep = ui_gtk_dispatch(event ? event->action : "idle", event ? event->payload : "");
    if (event) {
        g_free(event->action);
        g_free(event->payload);
        g_free(event);
    }
    if (keep && (ui_more_work || !g_queue_is_empty(&ui_events))) return G_SOURCE_CONTINUE;
    ui_idle_source = 0;
    return G_SOURCE_REMOVE;
}

static void ui_gtk_button_clicked(GtkButton *button, gpointer data) {
    ui_gtk_node *node = (ui_gtk_node *)data;
    (void)button;
    if (node) ui_gtk_enqueue("command", node->action);
}

/* Marshal owned facts using the existing catalogue payloads. Never call an
 * application from a widget signal, and never emit an input while rendering. */
static void ui_gtk_quote(GString *output, const char *text) {
    const unsigned char *cursor = (const unsigned char *)(text ? text : "");
    g_string_append_c(output, '"');
    for (; *cursor; ++cursor) {
        if (*cursor == '"' || *cursor == '\\') {
            g_string_append_c(output, '\\');
            g_string_append_c(output, (char)*cursor);
        } else if (*cursor < 32) {
            g_string_append_printf(output, "\\u%04x", (unsigned int)*cursor);
        } else g_string_append_c(output, (char)*cursor);
    }
    g_string_append_c(output, '"');
}

static void ui_gtk_choice_changed(GtkToggleButton *button, gpointer data) {
    ui_gtk_option *option = (ui_gtk_option *)data;
    GString *payload;
    if (ui_rendering || !option || !gtk_toggle_button_get_active(button)) return;
    payload = g_string_new("{\"node\":");
    ui_gtk_quote(payload, option->owner->id);
    g_string_append(payload, ",\"items\":[");
    ui_gtk_quote(payload, option->id);
    g_string_append(payload, "]}");
    ui_gtk_enqueue("selection.changed", payload->str);
    g_string_free(payload, TRUE);
}

static void ui_gtk_integer_fact(ui_gtk_node *node, const char *action) {
    GString *payload;
    if (ui_rendering || !node || !node->widget) return;
    payload = g_string_new("{\"node\":");
    ui_gtk_quote(payload, node->id);
    g_string_append(payload, ",\"value\":");
    ui_gtk_quote(payload, gtk_entry_get_text(GTK_ENTRY(node->widget)));
    g_string_append_c(payload, '}');
    ui_gtk_enqueue(action, payload->str);
    g_string_free(payload, TRUE);
}

static void ui_gtk_input_fact(ui_gtk_node *node, const char *action) {
    GString *payload;
    if (ui_rendering || !node || !node->widget) return;
    payload = g_string_new("{\"node\":");
    ui_gtk_quote(payload, node->id);
    g_string_append(payload, ",\"value\":");
    ui_gtk_quote(payload, gtk_entry_get_text(GTK_ENTRY(node->widget)));
    g_string_append_c(payload, '}');
    ui_gtk_enqueue(action, payload->str);
    g_string_free(payload, TRUE);
}

static void ui_gtk_input_changed(GtkEditable *editable, gpointer data) {
    (void)editable;
    ui_gtk_input_fact((ui_gtk_node *)data, "value.changed");
}

static void ui_gtk_input_committed(GtkEntry *entry, gpointer data) {
    (void)entry;
    ui_gtk_input_fact((ui_gtk_node *)data, "value.committed");
}

static void ui_gtk_checkbox_toggled(GtkToggleButton *button, gpointer data) {
    ui_gtk_node *node = (ui_gtk_node *)data;
    GString *payload;
    if (ui_rendering || !node) return;
    payload = g_string_new("{\"node\":");
    ui_gtk_quote(payload, node->id);
    g_string_append_printf(payload, ",\"checked\":%s}",
                           gtk_toggle_button_get_active(button) ? "true" : "false");
    ui_gtk_enqueue("checked.changed", payload->str);
    g_string_free(payload, TRUE);
}

static void ui_gtk_integer_changed(GtkEditable *editable, gpointer data) {
    (void)editable;
    ui_gtk_integer_fact((ui_gtk_node *)data, "value.changed");
}

static void ui_gtk_integer_committed(GtkEntry *entry, gpointer data) {
    (void)entry;
    ui_gtk_integer_fact((ui_gtk_node *)data, "value.committed");
}

static void ui_gtk_project_integer(ui_gtk_node *node, const char *text) {
    char *end;
    gint64 value;
    gdouble minimum, maximum;
    gtk_spin_button_get_range(GTK_SPIN_BUTTON(node->widget), &minimum, &maximum);
    errno = 0;
    value = g_ascii_strtoll(text, &end, 10);
    if (!errno && end != text && !*end && value >= minimum && value <= maximum)
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(node->widget), (gdouble)value);
    /* Keep transient/invalid edits as text; the feature owns validation. */
    gtk_entry_set_text(GTK_ENTRY(node->widget), text);
}

static gboolean ui_gtk_delete(GtkWidget *widget, GdkEvent *event, gpointer data) {
    (void)widget;
    (void)event;
    (void)data;
    ui_gtk_enqueue("close.requested", "");
    return TRUE; /* Only the eventual ui.close effect may destroy the surface. */
}

static void ui_gtk_destroy(GtkWidget *widget, gpointer data) {
    (void)data;
    if (widget == ui_window) {
        ui_window = NULL;
        ui_root = NULL;
        ui_grid = NULL;
        ui_gtk_clear_nodes();
        ui_gtk_enqueue("closed", "");
    }
}

static ui_gtk_node *ui_gtk_find_node(const char *id) {
    int index;
    for (index = 0; index < ui_node_count; ++index) {
        if (ui_nodes[index].id && strcmp(ui_nodes[index].id, id) == 0) {
            return &ui_nodes[index];
        }
    }
    return NULL;
}

PROCEDURE(create) {
    const char *title = GETSTRING(ARG0);
    int width = GETINT(ARG1);
    int height = GETINT(ARG2);

    if (ui_window || ui_active_loop) RETURNINTX(-1);
    if (!gtk_init_check(NULL, NULL)) RETURNINTX(0);

    ui_gtk_clear_nodes();
    ui_rendering = 0;
    ui_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    ui_root = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    ui_grid = gtk_grid_new();
    if (!ui_window || !ui_root || !ui_grid) RETURNINTX(0);

    gtk_window_set_title(GTK_WINDOW(ui_window), title);
    gtk_window_set_default_size(GTK_WINDOW(ui_window), width, height);
    gtk_widget_set_margin_top(ui_grid, 16);
    gtk_widget_set_margin_start(ui_grid, 16);
    gtk_widget_set_margin_end(ui_grid, 16);
    gtk_grid_set_row_spacing(GTK_GRID(ui_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(ui_grid), 16);
    gtk_box_pack_start(GTK_BOX(ui_root), ui_grid, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(ui_window), ui_root);
    g_signal_connect(ui_window, "delete-event", G_CALLBACK(ui_gtk_delete), NULL);
    g_signal_connect(ui_window, "destroy", G_CALLBACK(ui_gtk_destroy), NULL);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_label) {
    ui_gtk_node *node;
    const char *id = GETSTRING(ARG0);
    const char *text = GETSTRING(ARG1);
    int column = GETINT(ARG2);
    int row = GETINT(ARG3);

    if (!ui_grid || column < 0 || row < 0 ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id)) {
        RETURNINTX(0);
    }
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->widget = gtk_label_new(text);
    gtk_label_set_xalign(GTK_LABEL(node->widget), 0.0f);
    gtk_widget_set_halign(node->widget, GTK_ALIGN_START);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, column, row, 1, 1);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_statusbar) {
    ui_gtk_node *node;
    GtkWidget *frame;
    GtkWidget *message_area;
    const char *id = GETSTRING(ARG0);
    const char *text = GETSTRING(ARG1);
    int column = GETINT(ARG2);
    int row = GETINT(ARG3);

    if (!ui_grid || column < 0 || row < 0 ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id)) {
        RETURNINTX(0);
    }
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->widget = gtk_statusbar_new();
    node->status_context = gtk_statusbar_get_context_id(GTK_STATUSBAR(node->widget), id);
    gtk_statusbar_push(GTK_STATUSBAR(node->widget), node->status_context, text);
    message_area = gtk_statusbar_get_message_area(GTK_STATUSBAR(node->widget));
    gtk_widget_set_margin_top(message_area, 0);
    gtk_widget_set_margin_bottom(message_area, 0);
    gtk_widget_set_margin_start(message_area, 4);
    gtk_widget_set_margin_end(message_area, 4);
    gtk_widget_set_hexpand(node->widget, TRUE);
    gtk_widget_set_halign(node->widget, GTK_ALIGN_FILL);
    gtk_widget_set_size_request(node->widget, -1, 14);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(frame), node->widget);
    gtk_widget_set_hexpand(frame, TRUE);
    gtk_widget_set_halign(frame, GTK_ALIGN_FILL);
    gtk_widget_set_size_request(frame, -1, 18);
    gtk_box_pack_end(GTK_BOX(ui_root), frame, FALSE, FALSE, 0);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_line) {
    ui_gtk_node *node;
    const char *id = GETSTRING(ARG0);
    int column = GETINT(ARG1);
    int row = GETINT(ARG2);

    if (!ui_grid || column < 0 || row < 0 ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id)) {
        RETURNINTX(0);
    }
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->widget = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_hexpand(node->widget, TRUE);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, column, row, 1, 1);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_button) {
    ui_gtk_node *node;
    const char *id = GETSTRING(ARG0);
    const char *text = GETSTRING(ARG1);
    const char *action = GETSTRING(ARG2);
    int column = GETINT(ARG3);
    int row = GETINT(ARG4);

    if (!ui_grid || column < 0 || row < 0 ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id)) {
        RETURNINTX(0);
    }
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup(action);
    node->widget = gtk_button_new_with_label(text);
    g_signal_connect(node->widget, "clicked", G_CALLBACK(ui_gtk_button_clicked), node);
    gtk_widget_set_halign(node->widget, GTK_ALIGN_START);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, column, row, 1, 1);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_input) {
    ui_gtk_node *node;
    const char *id = GETSTRING(ARG0);
    const char *value = GETSTRING(ARG1);
    int column = GETINT(ARG2);
    int row = GETINT(ARG3);

    if (!ui_grid || column < 0 || row < 0 ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id)) {
        RETURNINTX(0);
    }
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->widget = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(node->widget), value);
    gtk_entry_set_width_chars(GTK_ENTRY(node->widget), 24);
    gtk_widget_set_hexpand(node->widget, TRUE);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, column, row, 1, 1);
    g_signal_connect(node->widget, "changed", G_CALLBACK(ui_gtk_input_changed), node);
    g_signal_connect(node->widget, "activate", G_CALLBACK(ui_gtk_input_committed), node);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_checkbox) {
    ui_gtk_node *node;
    const char *id = GETSTRING(ARG0), *text = GETSTRING(ARG1);
    gboolean checked = GETINT(ARG2) != 0;
    int column = GETINT(ARG3), row = GETINT(ARG4);
    if (!ui_grid || !*id || column < 0 || row < 0 ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id)) RETURNINTX(0);
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->widget = gtk_check_button_new_with_label(text);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(node->widget), checked);
    gtk_widget_set_halign(node->widget, GTK_ALIGN_START);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, column, row, 1, 1);
    g_signal_connect(node->widget, "toggled", G_CALLBACK(ui_gtk_checkbox_toggled), node);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_progress) {
    ui_gtk_node *node;
    const char *id = GETSTRING(ARG0);
    double value = g_ascii_strtod(GETSTRING(ARG1), NULL) / 100.0;
    int column = GETINT(ARG2), row = GETINT(ARG3);
    if (!ui_grid || !*id || column < 0 || row < 0 ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id)) RETURNINTX(0);
    if (value < 0.0) value = 0.0;
    if (value > 1.0) value = 1.0;
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->widget = gtk_progress_bar_new();
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(node->widget), value);
    gtk_widget_set_hexpand(node->widget, TRUE);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, column, row, 1, 1);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_choice) {
    const char *id = GETSTRING(ARG0);
    const char *selected = GETSTRING(ARG3);
    rxinteger column = GETINT(ARG4), row = GETINT(ARG5);
    size_t count = GETARRAYHI(ARG1), labels = GETARRAYHI(ARG2), index, previous;
    gboolean found = FALSE;
    GtkRadioButton *group = NULL;
    ui_gtk_node *node;
    if (!ui_grid || !*id || column < 0 || column > INT_MAX || row < 0 || row > INT_MAX ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id) ||
        count < 1 || count > 64 || count != labels) RETURNINTX(0);
    for (index = 0; index < count; ++index) {
        const char *item = GETSARRAY(ARG1, index);
        if (!item || !*item) RETURNINTX(0);
        if (strcmp(item, selected) == 0) found = TRUE;
        for (previous = 0; previous < index; ++previous)
            if (strcmp(item, GETSARRAY(ARG1, previous)) == 0) RETURNINTX(0);
    }
    if (!found) RETURNINTX(0);
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 3);
    node->options = g_new0(ui_gtk_option, count);
    node->option_count = (int)count;
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, (int)column, (int)row, 1, 1);
    g_object_ref(node->widget);
    node->owns_widget = TRUE;
    ++ui_rendering;
    for (index = 0; index < count; ++index) {
        ui_gtk_option *option = &node->options[index];
        option->id = g_strdup(GETSARRAY(ARG1, index));
        option->owner = node;
        option->widget = gtk_radio_button_new_with_label_from_widget(group, GETSARRAY(ARG2, index));
        if (!group) group = GTK_RADIO_BUTTON(option->widget);
        gtk_box_pack_start(GTK_BOX(node->widget), option->widget, FALSE, FALSE, 0);
        g_object_ref(option->widget);
        g_object_set_data(G_OBJECT(option->widget), "crexx-node-id", node->id);
        g_object_set_data(G_OBJECT(option->widget), "crexx-item-id", option->id);
        g_signal_connect(option->widget, "toggled", G_CALLBACK(ui_gtk_choice_changed), option);
    }
    for (index = 0; index < count; ++index)
        if (strcmp(node->options[index].id, selected) == 0)
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(node->options[index].widget), TRUE);
    --ui_rendering;
    RETURNINT(1);
    ENDPROC
}

static void ui_gtk_combo_changed(GtkComboBox *combo, gpointer data) {
    ui_gtk_node *node = (ui_gtk_node *)data;
    int index;
    GString *payload;
    if (ui_rendering || !node) return;
    index = gtk_combo_box_get_active(combo);
    if (index < 0 || index >= node->option_count) return;
    payload = g_string_new("{\"node\":");
    ui_gtk_quote(payload, node->id);
    g_string_append(payload, ",\"items\":[");
    ui_gtk_quote(payload, node->options[index].id);
    g_string_append(payload, "]}");
    ui_gtk_enqueue("selection.changed", payload->str);
    g_string_free(payload, TRUE);
}

PROCEDURE(add_combo) {
    const char *id = GETSTRING(ARG0), *selected = GETSTRING(ARG3);
    size_t count = GETARRAYHI(ARG1), index;
    ui_gtk_node *node;
    GtkComboBoxText *combo;
    int active = -1;
    if (!ui_grid || !*id || count < 1 || count > 64 || count != GETARRAYHI(ARG2) ||
        ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id)) RETURNINTX(0);
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->option_count = (int)count;
    node->options = g_new0(ui_gtk_option, count);
    combo = GTK_COMBO_BOX_TEXT(gtk_combo_box_text_new());
    node->widget = GTK_WIDGET(combo);
    for (index = 0; index < count; ++index) {
        node->options[index].id = g_strdup(GETSARRAY(ARG1, index));
        node->options[index].owner = node;
        gtk_combo_box_text_append(combo, node->options[index].id, GETSARRAY(ARG2, index));
        if (strcmp(node->options[index].id, selected) == 0) active = (int)index;
    }
    if (active < 0) RETURNINTX(0);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), active);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, GETINT(ARG4), GETINT(ARG5), 1, 1);
    g_signal_connect(node->widget, "changed", G_CALLBACK(ui_gtk_combo_changed), node);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(set_combo) {
    ui_gtk_node *node = ui_gtk_find_node(GETSTRING(ARG0));
    const char *selected = GETSTRING(ARG3);
    int index;
    if (!node || !GTK_IS_COMBO_BOX(node->widget)) RETURNINTX(0);
    for (index = 0; index < node->option_count; ++index)
        if (strcmp(node->options[index].id, selected) == 0) break;
    if (index >= node->option_count) RETURNINTX(0);
    ++ui_rendering;
    gtk_combo_box_set_active(GTK_COMBO_BOX(node->widget), index);
    --ui_rendering;
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(set_choice) {
    ui_gtk_node *node = ui_gtk_find_node(GETSTRING(ARG0));
    const char *selected = GETSTRING(ARG3);
    size_t count = GETARRAYHI(ARG1), index;
    gboolean found = FALSE;
    if (!node || !node->options || count != (size_t)node->option_count ||
        count != GETARRAYHI(ARG2)) RETURNINTX(0);
    for (index = 0; index < count; ++index) {
        if (strcmp(node->options[index].id, GETSARRAY(ARG1, index)) != 0) RETURNINTX(0);
        if (strcmp(node->options[index].id, selected) == 0) found = TRUE;
    }
    if (!found) RETURNINTX(0);
    /* Later input facts already own the user's current control state. Do not
     * reproject an earlier owner turn over it; the last mailbox turn renders
     * the final model once every captured fact has been admitted. */
    if (!g_queue_is_empty(&ui_events)) RETURNINTX(1);
    ++ui_rendering;
    for (index = 0; index < count; ++index) {
        gtk_button_set_label(GTK_BUTTON(node->options[index].widget), GETSARRAY(ARG2, index));
        if (strcmp(node->options[index].id, selected) == 0)
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(node->options[index].widget), TRUE);
    }
    --ui_rendering;
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(add_integer) {
    const char *id = GETSTRING(ARG0), *value = GETSTRING(ARG1);
    rxinteger minimum = GETINT(ARG2), maximum = GETINT(ARG3), step = GETINT(ARG4);
    rxinteger column = GETINT(ARG5), row = GETINT(ARG6);
    ui_gtk_node *node;
    /* GTK's integer getter and platform coordinates are signed 32-bit. */
    if (!ui_grid || !*id || ui_node_count >= UI_GTK_MAX_NODES || ui_gtk_find_node(id) ||
        minimum < INT_MIN || maximum > INT_MAX || minimum > maximum || step < 1 || step > INT_MAX ||
        column < 0 || column > INT_MAX || row < 0 || row > INT_MAX) RETURNINTX(0);
    node = &ui_nodes[ui_node_count++];
    node->id = g_strdup(id);
    node->action = g_strdup("");
    node->widget = gtk_spin_button_new_with_range((gdouble)minimum, (gdouble)maximum, (gdouble)step);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(node->widget), 0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(node->widget), TRUE);
    gtk_spin_button_set_update_policy(GTK_SPIN_BUTTON(node->widget), GTK_UPDATE_IF_VALID);
    gtk_entry_set_width_chars(GTK_ENTRY(node->widget), 8);
    g_object_set_data(G_OBJECT(node->widget), "crexx-node-id", node->id);
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, (int)column, (int)row, 1, 1);
    g_object_ref(node->widget);
    node->owns_widget = TRUE;
    ++ui_rendering;
    ui_gtk_project_integer(node, value);
    --ui_rendering;
    g_signal_connect(node->widget, "changed", G_CALLBACK(ui_gtk_integer_changed), node);
    g_signal_connect(node->widget, "activate", G_CALLBACK(ui_gtk_integer_committed), node);
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(set_integer) {
    ui_gtk_node *node = ui_gtk_find_node(GETSTRING(ARG0));
    const char *value = GETSTRING(ARG1);
    if (!node || !node->widget || !GTK_IS_SPIN_BUTTON(node->widget)) RETURNINTX(0);
    if (!g_queue_is_empty(&ui_events)) RETURNINTX(1);
    ++ui_rendering;
    if (strcmp(gtk_entry_get_text(GTK_ENTRY(node->widget)), value) != 0)
        ui_gtk_project_integer(node, value);
    --ui_rendering;
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(set_text) {
    const char *id = GETSTRING(ARG0);
    const char *text = GETSTRING(ARG1);
    ui_gtk_node *node = ui_gtk_find_node(id);

    if (!node || !node->widget) RETURNINTX(0);
    if (GTK_IS_LABEL(node->widget)) {
        gtk_label_set_text(GTK_LABEL(node->widget), text);
    } else if (GTK_IS_STATUSBAR(node->widget)) {
        gtk_statusbar_pop(GTK_STATUSBAR(node->widget), node->status_context);
        gtk_statusbar_push(GTK_STATUSBAR(node->widget), node->status_context, text);
    } else if (GTK_IS_PROGRESS_BAR(node->widget)) {
        double value = g_ascii_strtod(text, NULL) / 100.0;
        if (value < 0.0) value = 0.0;
        if (value > 1.0) value = 1.0;
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(node->widget), value);
    } else if (GTK_IS_CHECK_BUTTON(node->widget)) {
        gtk_button_set_label(GTK_BUTTON(node->widget), text);
    } else if (GTK_IS_BUTTON(node->widget)) {
        gtk_button_set_label(GTK_BUTTON(node->widget), text);
    } else if (GTK_IS_ENTRY(node->widget)) {
        if (strcmp(gtk_entry_get_text(GTK_ENTRY(node->widget)), text) != 0) {
            ++ui_rendering;
            gtk_entry_set_text(GTK_ENTRY(node->widget), text);
            --ui_rendering;
        }
    } else {
        RETURNINTX(0);
    }
    RETURNINT(1);
    ENDPROC
}

PROCEDURE(show) {
    if (!ui_window) RETURNINTX(0);
    gtk_widget_show_all(ui_window);
    RETURNINT(1);
    ENDPROC
}


static void ui_gtk_dialog_response(GtkDialog *dialog, gint response, gpointer ignored) {
    char *filename = NULL;
    gboolean accepted = response == GTK_RESPONSE_ACCEPT || response == GTK_RESPONSE_OK;
    (void)ignored;
    if (accepted && ui_dialog_is_file)
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    if (ui_dialog_is_file && !filename) accepted = FALSE;
    ui_dialog = NULL;
    ui_gtk_enqueue(accepted ? "dialog.accepted" : "dialog.cancelled", filename);
    gtk_widget_destroy(GTK_WIDGET(dialog));
    g_free(filename);
}

PROCEDURE(begin_dialog) {
    const char *kind = GETSTRING(ARG0);
    const char *title = GETSTRING(ARG1);
    const char *message = GETSTRING(ARG2);
    if (!ui_window || ui_dialog) RETURNINTX(0);
    ui_dialog_is_file = strcmp(kind, "file") == 0 || strcmp(kind, "save") == 0;
    if (ui_dialog_is_file) {
        ui_dialog = gtk_file_chooser_dialog_new(title, GTK_WINDOW(ui_window),
                strcmp(kind, "save") == 0 ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
                "_Cancel", GTK_RESPONSE_CANCEL,
                strcmp(kind, "save") == 0 ? "_Save" : "_Open", GTK_RESPONSE_ACCEPT, NULL);
        /* The shared feature requests its own correlated confirmation. */
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(ui_dialog), FALSE);
        gtk_window_set_default_size(GTK_WINDOW(ui_dialog), 760, 520);
    } else if (strcmp(kind, "confirm") == 0) {
        ui_dialog = gtk_message_dialog_new(GTK_WINDOW(ui_window),
                GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
                "%s", message);
        gtk_window_set_title(GTK_WINDOW(ui_dialog), title);
    } else RETURNINTX(0);
    gtk_window_set_modal(GTK_WINDOW(ui_dialog), TRUE);
    g_signal_connect(ui_dialog, "response", G_CALLBACK(ui_gtk_dialog_response), NULL);
    gtk_widget_show_all(ui_dialog);
    RETURNINT(1);
    ENDPROC
}

/* Explicit close effect, separate from the vetoable window-manager intent. */
PROCEDURE(close_surface) {
    if (ui_dialog) gtk_dialog_response(GTK_DIALOG(ui_dialog), GTK_RESPONSE_CANCEL);
    if (ui_window) gtk_widget_destroy(ui_window);
    ENDPROC
}

PROCEDURE(run) {
    ui_gtk_loop state;

    if (!ui_window || ui_active_loop || !ISINITIALIZED(ARG0)) RETURNINTX(-1);
    memset(&state, 0, sizeof(state));
    state.handler = ARG0;
    state.scratch_and_result = RETURN;
    state.signal = SIGNAL;
    ui_active_loop = &state;

    ui_more_work = 0;
    ui_gtk_enqueue("ready", "");
    gtk_main();
    ui_active_loop = NULL;
    if (ui_idle_source) g_source_remove(ui_idle_source);
    ui_idle_source = 0;
    ui_gtk_discard_events();

    if (state.callback_failed) {
        if (GETINT(SIGNAL) == SIGNAL_NONE) {
            RETURNSIGNAL(SIGNAL_FAILURE, "ui_gtk_native.run callback failed")
        }
        return;
    }
    RETURNINT(state.event_count);
    ENDPROC
}

PROCEDURE(cleanup) {
    if (ui_dialog) {
        gtk_widget_destroy(ui_dialog);
        ui_dialog = NULL;
    }
    if (ui_window) gtk_widget_destroy(ui_window);
    ui_window = NULL;
    ui_root = NULL;
    ui_grid = NULL;
    ui_gtk_clear_nodes();
    ENDPROC
}

LOADFUNCS
    ADDINTERFACE("ui_gtk_native.eventhandler");
    ADDMETHOD("ui_gtk_native.eventhandler", "on_native_event", ".int",
              "action=.string,payload=.string");
    ADDPROC(create, "ui_gtk_native.create", "b", ".int",
            "title=.string,width=.int,height=.int");
    ADDPROC(add_label, "ui_gtk_native.add_label", "b", ".int",
            "id=.string,text=.string,column=.int,row=.int");
    ADDPROC(add_statusbar, "ui_gtk_native.add_statusbar", "b", ".int",
            "id=.string,text=.string,column=.int,row=.int");
    ADDPROC(add_line, "ui_gtk_native.add_line", "b", ".int",
            "id=.string,column=.int,row=.int");
    ADDPROC(add_button, "ui_gtk_native.add_button", "b", ".int",
            "id=.string,text=.string,action=.string,column=.int,row=.int");
    ADDPROC(add_input, "ui_gtk_native.add_input", "b", ".int",
            "id=.string,text=.string,column=.int,row=.int");
    ADDPROC(add_checkbox, "ui_gtk_native.add_checkbox", "b", ".int",
            "id=.string,text=.string,checked=.int,column=.int,row=.int");
    ADDPROC(add_progress, "ui_gtk_native.add_progress", "b", ".int",
            "id=.string,value=.string,column=.int,row=.int");
    ADDPROC(set_text, "ui_gtk_native.set_text", "b", ".int",
            "id=.string,text=.string");
    ADDPROC(add_choice, "ui_gtk_native.add_choice", "b", ".int",
            "id=.string,option_ids=.string[],option_texts=.string[],selected=.string,column=.int,row=.int");
    ADDPROC(add_combo, "ui_gtk_native.add_combo", "b", ".int",
            "id=.string,option_ids=.string[],option_texts=.string[],selected=.string,column=.int,row=.int");
    ADDPROC(set_combo, "ui_gtk_native.set_combo", "b", ".int",
            "id=.string,option_ids=.string[],option_texts=.string[],selected=.string");
    ADDPROC(set_choice, "ui_gtk_native.set_choice", "b", ".int",
            "id=.string,option_ids=.string[],option_texts=.string[],selected=.string");
    ADDPROC(add_integer, "ui_gtk_native.add_integer", "b", ".int",
            "id=.string,value=.string,minimum=.int,maximum=.int,step=.int,column=.int,row=.int");
    ADDPROC(set_integer, "ui_gtk_native.set_integer", "b", ".int",
            "id=.string,value=.string");
    ADDPROC(show, "ui_gtk_native.show", "b", ".int", "");
    ADDPROC(begin_dialog, "ui_gtk_native.begin_dialog", "b", ".int",
            "kind=.string,title=.string,message=.string");
    ADDPROC(close_surface, "ui_gtk_native.close_surface", "b", ".void", "");
    ADDPROC(run, "ui_gtk_native.run", "b", ".int", "handler=.eventhandler");
    ADDPROC(cleanup, "ui_gtk_native.cleanup", "b", ".void", "");
ENDLOADFUNCS
