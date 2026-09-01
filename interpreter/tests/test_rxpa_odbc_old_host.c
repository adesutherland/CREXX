/* Prove that a host which only knows _initfuncs can use the default session. */
#include "rxpa.h"

#include <stdio.h>
#include <string.h>

typedef struct legacy_value {
    char string[256];
    rxinteger integer;
    double floating;
} legacy_value;

static rxpa_libfunc legacy_connect;
static rxpa_libfunc legacy_disconnect;

static void legacy_addfunc(rxpa_libfunc function, char *name, char *option,
                           char *type, char *args) {
    (void)option; (void)type; (void)args;
    if (strcmp(name, "odbc.odbc_connect") == 0) legacy_connect = function;
    if (strcmp(name, "odbc.odbc_disconnect") == 0) legacy_disconnect = function;
}

static void legacy_addclass(char *name, char *option, char *type) {
    (void)name; (void)option; (void)type;
}
static void legacy_addinterface(char *name, char *option, char *type) {
    (void)name; (void)option; (void)type;
}
static void legacy_addimplements(char *name, char *interface_name) {
    (void)name; (void)interface_name;
}
static void legacy_addmember(char *owner, char *kind, char *member,
                             char *type, char *args) {
    (void)owner; (void)kind; (void)member; (void)type; (void)args;
}
static char *legacy_getstring(rxpa_attribute_value value) {
    return ((legacy_value *)value)->string;
}
static void legacy_setstring(rxpa_attribute_value value, const char *text) {
    snprintf(((legacy_value *)value)->string,
             sizeof(((legacy_value *)value)->string), "%s", text ? text : "");
}
static void legacy_setint(rxpa_attribute_value value, rxinteger integer) {
    ((legacy_value *)value)->integer = integer;
}
static rxinteger legacy_getint(rxpa_attribute_value value) {
    return ((legacy_value *)value)->integer;
}
static void legacy_setfloat(rxpa_attribute_value value, double floating) {
    ((legacy_value *)value)->floating = floating;
}
static double legacy_getfloat(rxpa_attribute_value value) {
    return ((legacy_value *)value)->floating;
}
static int legacy_setnativepayload(rxpa_attribute_value value,
                                   const void *payload, size_t length,
                                   const rxpa_native_payload_ops *ops,
                                   unsigned int flags) {
    (void)value; (void)payload; (void)length; (void)ops; (void)flags;
    return -1;
}
static void *legacy_getnativepayload(rxpa_attribute_value value,
                                     size_t *length,
                                     const rxpa_native_payload_ops **ops,
                                     unsigned int *flags) {
    (void)value;
    if (length) *length = 0u;
    if (ops) *ops = NULL;
    if (flags) *flags = 0u;
    return NULL;
}
static rxinteger legacy_getnumattrs(rxpa_attribute_value value) {
    (void)value; return 0;
}
static void legacy_setnumattrs(rxpa_attribute_value value,
                               rxinteger number) {
    (void)value; (void)number;
}
static rxpa_attribute_value legacy_getattr(rxpa_attribute_value value,
                                           rxinteger index) {
    (void)value; (void)index; return NULL;
}
static rxpa_attribute_value legacy_insertattr(rxpa_attribute_value value,
                                              rxinteger index) {
    (void)value; (void)index; return NULL;
}
static void legacy_removeattr(rxpa_attribute_value value, rxinteger index) {
    (void)value; (void)index;
}
static void legacy_swapattrs(rxpa_attribute_value value, rxinteger first,
                             rxinteger second) {
    (void)value; (void)first; (void)second;
}
static void legacy_setsayexit(say_exit_func function) { (void)function; }
static void legacy_resetsayexit(void) {}

int main(int argc, char **argv) {
    rxpa_initctx helpers;
    legacy_value dsn = {{0}, 0, 0.0};
    legacy_value user = {{0}, 0, 0.0};
    legacy_value password = {{0}, 0, 0.0};
    legacy_value result = {{0}, 0, 0.0};
    legacy_value signal = {{0}, 0, 0.0};
    rxpa_attribute_value arguments[3];
    int rc;
    if (argc != 2) return 1;
    memset(&helpers, 0, sizeof(helpers));
    helpers.addfunc = legacy_addfunc;
    helpers.addclass = legacy_addclass;
    helpers.addinterface = legacy_addinterface;
    helpers.addimplements = legacy_addimplements;
    helpers.addmember = legacy_addmember;
    helpers.getstring = legacy_getstring;
    helpers.setstring = legacy_setstring;
    helpers.setint = legacy_setint;
    helpers.getint = legacy_getint;
    helpers.setfloat = legacy_setfloat;
    helpers.getfloat = legacy_getfloat;
    helpers.setnativepayload = legacy_setnativepayload;
    helpers.getnativepayload = legacy_getnativepayload;
    helpers.getnumattrs = legacy_getnumattrs;
    helpers.setnumattrs = legacy_setnumattrs;
    helpers.getattr = legacy_getattr;
    helpers.insertattr = legacy_insertattr;
    helpers.removeattr = legacy_removeattr;
    helpers.swapattrs = legacy_swapattrs;
    helpers.setsayexit = legacy_setsayexit;
    helpers.resetsayexit = legacy_resetsayexit;
    rc = load_plugin(&helpers, argv[1], "rx_odbc.rxplugin");
    if (rc != 0 || !legacy_connect || !legacy_disconnect) {
        fprintf(stderr, "Legacy host did not receive ODBC procedures\n");
        return 1;
    }
    snprintf(dsn.string, sizeof(dsn.string), "legacy-default");
    arguments[0] = &dsn;
    arguments[1] = &user;
    arguments[2] = &password;
    legacy_connect(3, arguments, &result, &signal);
    if (result.integer != 0 || signal.integer != SIGNAL_NONE) {
        fprintf(stderr, "Legacy default-session connect failed\n");
        return 1;
    }
    legacy_disconnect(0, NULL, &result, &signal);
    if (result.integer != 0 || signal.integer != SIGNAL_NONE) {
        fprintf(stderr, "Legacy default-session disconnect failed\n");
        return 1;
    }
    puts("ODBC_OLD_HOST_DEFAULT_SESSION_OK");
    return 0;
}
