/* Test-only GTK input injector. Uses real widget signals and dialog responses;
 * it imports no production host internals and is never installed. A finite
 * timeout bounds missing state; progress depends on observed widgets, not sleeps.
 */
#include <gtk/gtk.h>
#include <string.h>
#include "crexxpa.h"

static gchar **steps;
static int step_count, cursor, ticks, result;
static guint source;
static guint commit_source;
static gboolean choosing;

static uint32_t test_capabilities(const char *name) { (void)name; return 0u; }
RXPA_PLUGIN_PROCEDURE_CAPABILITIES(test_capabilities)

static GtkWidget *find_node(GtkWidget *widget, const char *id) {
    const char *node_id = g_object_get_data(G_OBJECT(widget), "crexx-node-id");
    GtkWidget *found = NULL;
    if (node_id && strcmp(node_id, id) == 0) return widget;
    if (GTK_IS_CONTAINER(widget)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget)), *p;
        for (p = children; p && !found; p = p->next) found = find_node(p->data, id);
        g_list_free(children);
    }
    return found;
}

static GtkWidget *find_item(GtkWidget *widget, const char *node, const char *item) {
    const char *node_id = g_object_get_data(G_OBJECT(widget), "crexx-node-id");
    const char *item_id = g_object_get_data(G_OBJECT(widget), "crexx-item-id");
    GtkWidget *found = NULL;
    if (node_id && item_id && !strcmp(node_id, node) && !strcmp(item_id, item)) return widget;
    if (GTK_IS_CONTAINER(widget)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(widget)), *p;
        for (p = children; p && !found; p = p->next) found = find_item(p->data, node, item);
        g_list_free(children);
    }
    return found;
}

static gboolean exclusive_item(GtkWidget *item) {
    GSList *p;
    int active = 0;
    if (!item || !GTK_IS_RADIO_BUTTON(item)) return FALSE;
    for (p = gtk_radio_button_get_group(GTK_RADIO_BUTTON(item)); p; p = p->next)
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->data))) ++active;
    return active == 1 && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(item));
}

static gboolean commit_queued_edit(gpointer entry) {
    commit_source = 0;
    g_signal_emit_by_name(entry, "activate");
    return G_SOURCE_REMOVE;
}

static gboolean drive(gpointer ignored) {
    GList *windows = gtk_window_list_toplevels(), *p;
    GtkWidget *window = NULL, *dialog = NULL, *node;
    gchar **parts;
    gboolean done = FALSE;
    (void)ignored;
    for (p = windows; p; p = p->next) {
        if (!gtk_widget_get_visible(p->data)) continue;
        if (GTK_IS_DIALOG(p->data)) dialog = p->data;
        else if (GTK_IS_WINDOW(p->data) && !gtk_window_get_transient_for(GTK_WINDOW(p->data)))
            window = p->data;
    }
    g_list_free(windows);
    if (cursor == step_count) { result = 0; source = 0; return G_SOURCE_REMOVE; }
    if (++ticks > 2000) {
        g_printerr("GTK scenario timed out at step %d: %s\n", cursor, steps[cursor]);
        result = -1; source = 0;
        if (gtk_main_level()) gtk_main_quit();
        return G_SOURCE_REMOVE;
    }
    parts = g_strsplit(steps[cursor], "|", 3);
    if (!strcmp(parts[0], "click") && parts[1] && window && !dialog) {
        node = find_node(window, parts[1]);
        if (node && GTK_IS_BUTTON(node)) { gtk_button_clicked(GTK_BUTTON(node)); done = TRUE; }
    } else if (!strcmp(parts[0], "choice") && parts[1] && parts[2] && window && !dialog) {
        node = find_item(window, parts[1], parts[2]);
        if (node && GTK_IS_RADIO_BUTTON(node)) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(node), TRUE); done = TRUE;
        }
    } else if (!strcmp(parts[0], "choice-edit-commit") && parts[1] && parts[2] && window && !dialog) {
        gchar **edit = g_strsplit(parts[2], "|", 3);
        if (edit[0] && edit[1] && edit[2]) {
            GtkWidget *choice = find_item(window, parts[1], edit[0]);
            GtkWidget *entry = find_node(window, edit[1]);
            if (choice && GTK_IS_RADIO_BUTTON(choice) && entry && GTK_IS_ENTRY(entry)) {
                /* Two real input facts before the owner idle turn. A render
                 * for the first must not overwrite the second queued edit. */
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(choice), TRUE);
                gtk_entry_set_text(GTK_ENTRY(entry), edit[2]);
                /* Commit after one owner idle turn, while the second fact can
                 * still be pending. Use real activation and an owned widget. */
                commit_source = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
                        commit_queued_edit, g_object_ref(entry), g_object_unref);
                done = TRUE;
            }
        }
        g_strfreev(edit);
    } else if (!strcmp(parts[0], "expect-choice") && parts[1] && parts[2] && window) {
        done = exclusive_item(find_item(window, parts[1], parts[2]));
    } else if (!strcmp(parts[0], "value") && parts[1] && parts[2] && window && !dialog) {
        node = find_node(window, parts[1]);
        if (node && GTK_IS_ENTRY(node)) { gtk_entry_set_text(GTK_ENTRY(node), parts[2]); done = TRUE; }
    } else if (!strcmp(parts[0], "expect-value") && parts[1] && parts[2] && window) {
        node = find_node(window, parts[1]);
        if (node && GTK_IS_ENTRY(node)) done = !strcmp(gtk_entry_get_text(GTK_ENTRY(node)), parts[2]);
    } else if (!strcmp(parts[0], "enter") && parts[1] && window && !dialog) {
        node = find_node(window, parts[1]);
        if (node && GTK_IS_ENTRY(node)) { g_signal_emit_by_name(node, "activate"); done = TRUE; }
    } else if (!strcmp(parts[0], "spin") && parts[1] && parts[2] && window && !dialog) {
        node = find_node(window, parts[1]);
        if (node && GTK_IS_SPIN_BUTTON(node)) {
            gtk_spin_button_spin(GTK_SPIN_BUTTON(node), GTK_SPIN_STEP_FORWARD, g_ascii_strtod(parts[2], NULL));
            done = TRUE;
        }
    } else if (!strcmp(parts[0], "expect") && parts[1] && parts[2] && window) {
        node = find_node(window, parts[1]);
        if (node && GTK_IS_LABEL(node)) done = !strcmp(gtk_label_get_text(GTK_LABEL(node)), parts[2]);
    } else if (!strcmp(parts[0], "cancel") && dialog) {
        gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL); done = TRUE;
    } else if (!strcmp(parts[0], "ok") && dialog && !GTK_IS_FILE_CHOOSER(dialog)) {
        gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); done = TRUE;
    } else if (!strcmp(parts[0], "file") && parts[1] && dialog && GTK_IS_FILE_CHOOSER(dialog)) {
        gchar *selected;
        if (!choosing) {
            choosing = TRUE;
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), parts[1]);
        }
        selected = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (selected) {
            GFile *expected_file = g_file_new_for_path(parts[1]);
            GFile *selected_file = g_file_new_for_path(selected);
            /* Windows returns backslashes even for a forward-slash CMake
             * argument. Compare file identities rather than display strings. */
            if (g_file_equal(expected_file, selected_file)) {
                gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT); done = TRUE;
            }
            g_object_unref(selected_file);
            g_object_unref(expected_file);
        }
        g_free(selected);
    } else if (!strcmp(parts[0], "close") && window && !dialog) {
        gboolean handled = FALSE;
        GdkEvent *event = gdk_event_new(GDK_DELETE);
        g_signal_emit_by_name(window, "delete-event", event, &handled);
        gdk_event_free(event);
        done = handled;
    }
    g_strfreev(parts);
    if (done) { ++cursor; ticks = 0; choosing = FALSE; }
    return G_SOURCE_CONTINUE;
}

PROCEDURE(start) {
    int i;
    if (source) RETURNINTX(-1);
    g_strfreev(steps);
    step_count = GETNUMATTRS(ARG0);
    if (step_count < 1 || step_count > 64) { steps = NULL; RETURNINTX(-1); }
    steps = g_new0(gchar *, step_count + 1);
    for (i = 0; i < step_count; ++i) steps[i] = g_strdup(GETSTRING(GETATTR(ARG0, i)));
    cursor = ticks = 0; result = 1; choosing = FALSE;
    source = g_timeout_add(10, drive, NULL);
    RETURNINT(0);
    ENDPROC
}

PROCEDURE(finish) {
    if (source) g_source_remove(source);
    source = 0;
    if (commit_source) g_source_remove(commit_source);
    commit_source = 0;
    /* The final close may end GTK before the timer observes the next tick. */
    if (cursor == step_count) result = 0;
    g_strfreev(steps); steps = NULL;
    RETURNINT(result);
    ENDPROC
}

LOADFUNCS
    ADDPROC(start, "ui_gtk_test.start", "b", ".int", "steps=.string[]");
    ADDPROC(finish, "ui_gtk_test.finish", "b", ".int", "");
ENDLOADFUNCS
