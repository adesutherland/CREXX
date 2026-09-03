/* Minimal GTK 3 driver for the experimental cREXX UI library. */
#include <gtk/gtk.h>
#include <string.h>

#include "crexxpa.h"

#define UI_GTK_MAX_NODES 64

typedef struct ui_gtk_node {
    char *id;
    char *action;
    GtkWidget *widget;
} ui_gtk_node;

typedef struct ui_gtk_loop {
    rxpa_attribute_value handler;
    rxpa_attribute_value scratch_and_result;
    rxpa_attribute_value signal;
    int event_count;
    gboolean callback_failed;
} ui_gtk_loop;

static GtkWidget *ui_window;
static GtkWidget *ui_grid;
static ui_gtk_node ui_nodes[UI_GTK_MAX_NODES];
static int ui_node_count;
static ui_gtk_loop *ui_active_loop;
static const char ui_handler_descriptor[] =
        "rxsig1|on_native_event|.int|action=.string,payload=.string";

static void ui_gtk_clear_nodes(void) {
    int index;
    for (index = 0; index < UI_GTK_MAX_NODES; ++index) {
        g_free(ui_nodes[index].id);
        g_free(ui_nodes[index].action);
        ui_nodes[index].id = NULL;
        ui_nodes[index].action = NULL;
        ui_nodes[index].widget = NULL;
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
    if (GETINT(ui_active_loop->scratch_and_result) == 0) {
        if (gtk_main_level() > 0) gtk_main_quit();
        return FALSE;
    }
    return TRUE;
}

static gboolean ui_gtk_ready(gpointer ignored) {
    (void)ignored;
    (void)ui_gtk_dispatch("app.ready", "");
    return G_SOURCE_REMOVE;
}

static void ui_gtk_button_clicked(GtkButton *button, gpointer data) {
    ui_gtk_node *node = (ui_gtk_node *)data;
    (void)button;
    if (node) (void)ui_gtk_dispatch(node->action, "");
}

static gboolean ui_gtk_delete(GtkWidget *widget, GdkEvent *event, gpointer data) {
    (void)widget;
    (void)event;
    (void)data;
    (void)ui_gtk_dispatch("app.quit.requested", "");
    if (gtk_main_level() > 0) gtk_main_quit();
    return FALSE;
}

static void ui_gtk_destroy(GtkWidget *widget, gpointer data) {
    (void)data;
    if (widget == ui_window) {
        ui_window = NULL;
        ui_grid = NULL;
        ui_gtk_clear_nodes();
        if (ui_active_loop && gtk_main_level() > 0) gtk_main_quit();
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
    ui_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    ui_grid = gtk_grid_new();
    if (!ui_window || !ui_grid) RETURNINTX(0);

    gtk_window_set_title(GTK_WINDOW(ui_window), title);
    gtk_window_set_default_size(GTK_WINDOW(ui_window), width, height);
    gtk_container_set_border_width(GTK_CONTAINER(ui_window), 16);
    gtk_grid_set_row_spacing(GTK_GRID(ui_grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(ui_grid), 16);
    gtk_container_add(GTK_CONTAINER(ui_window), ui_grid);
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
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, column, row, 1, 1);
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
    gtk_grid_attach(GTK_GRID(ui_grid), node->widget, column, row, 1, 1);
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
    } else if (GTK_IS_BUTTON(node->widget)) {
        gtk_button_set_label(GTK_BUTTON(node->widget), text);
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

PROCEDURE(choose_file) {
    GtkWidget *dialog;
    char *filename = NULL;

    if (!ui_window) RETURNSTRX("");
    dialog = gtk_file_chooser_dialog_new(
            GETSTRING(ARG0),
            GTK_WINDOW(ui_window),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Open", GTK_RESPONSE_ACCEPT,
            NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 760, 520);
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    }
    gtk_widget_destroy(dialog);
    if (!filename) RETURNSTRX("");
    SETSTRING(RETURN, filename);
    g_free(filename);
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

    if (g_idle_add(ui_gtk_ready, NULL) == 0) {
        ui_active_loop = NULL;
        RETURNSIGNAL(SIGNAL_FAILURE, "ui_gtk_native.run could not schedule app.ready")
    }
    gtk_main();
    ui_active_loop = NULL;

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
    if (ui_window) gtk_widget_destroy(ui_window);
    ui_window = NULL;
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
    ADDPROC(add_line, "ui_gtk_native.add_line", "b", ".int",
            "id=.string,column=.int,row=.int");
    ADDPROC(add_button, "ui_gtk_native.add_button", "b", ".int",
            "id=.string,text=.string,action=.string,column=.int,row=.int");
    ADDPROC(set_text, "ui_gtk_native.set_text", "b", ".int",
            "id=.string,text=.string");
    ADDPROC(show, "ui_gtk_native.show", "b", ".int", "");
    ADDPROC(choose_file, "ui_gtk_native.choose_file", "b", ".string",
            "title=.string");
    ADDPROC(run, "ui_gtk_native.run", "b", ".int", "handler=.eventhandler");
    ADDPROC(cleanup, "ui_gtk_native.cleanup", "b", ".void", "");
ENDLOADFUNCS
