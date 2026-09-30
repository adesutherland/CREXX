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
static gboolean choosing;

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

static gboolean drive(gpointer ignored) {
    GList *windows = gtk_window_list_toplevels(), *p;
    GtkWidget *window = NULL, *dialog = NULL, *node;
    gchar **parts;
    gboolean done = FALSE;
    (void)ignored;
    for (p = windows; p; p = p->next) {
        if (!gtk_widget_get_visible(p->data)) continue;
        if (GTK_IS_DIALOG(p->data)) dialog = p->data;
        else if (find_node(p->data, "open")) window = p->data;
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
        if (selected && !strcmp(selected, parts[1])) {
            gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT); done = TRUE;
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
