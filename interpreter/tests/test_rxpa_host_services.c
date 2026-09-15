/* S4-D01: complete borrowed text and negotiated session factories. */
#define main rxpa_concurrency_fixture_main
#include "test_rxpa_concurrency.c"
#undef main

static const rxpa_host_services_v1 *received_host;
void rxvm_addclass(char *, char *, char *);
void rxvm_addinterface(char *, char *, char *);
static int legacy_creates, host_creates, destroys;
static void *legacy_create(void) { ++legacy_creates; return &legacy_creates; }
static void *host_create(const rxpa_host_services_v1 *host) {
    if (!rxpa_host_has_string_view(host)) return NULL;
    ++host_creates; received_host = host; return &host_creates;
}
static void destroy(void *session) { if (session) ++destroys; }
static void reject_uninitialized(rxinteger n, rxpa_attribute_value *args,
                                 rxpa_attribute_value ret, rxpa_attribute_value sig) {
    (void)n; (void)args;
    set_int(ret, 0);
    set_int(sig, SIGNAL_OBJECT_NOT_INITIALIZED);
}
static void probe(rxinteger n, rxpa_attribute_value *args,
                  rxpa_attribute_value ret, rxpa_attribute_value sig) {
    const char *data = NULL;
    size_t length = 999;
    static const char text[] = {'A', 0, 'B', (char)0xc3, (char)0xa9};
    (void)sig;
    if (n != 1 || received_host->string_view(args[0], &data, &length) ||
        length != sizeof(text) || memcmp(data, text, sizeof(text)) ||
        data != ((value *)args[0])->string_value) {
        set_int(ret, 1); return;
    }
    /* Empty text is successful; no terminator or writable buffer is promised. */
    set_string(args[0], "", 0);
    if (received_host->string_view(args[0], &data, &length) || length) {
        set_int(ret, 2); return;
    }
    if (!received_host->string_view(NULL, &data, &length) ||
        !received_host->string_view(args[0], NULL, &length) ||
        !received_host->string_view(args[0], &data, NULL)) {
        set_int(ret, 3); return;
    }
    {
        value *object = args[0];
        const RxGraphTypeRef *identity;
        uint32_t before;
        set_int(object, 73);
        set_num_attributes(object, 1);
        set_int(object->attributes[0], 91);
        if (set_native_payload(object, text, sizeof(text), NULL, 0)) {
            set_int(ret, 4); return;
        }
        mark_value_uninitialized_object(object);
        before = object->status.all_type_flags;
        if (SETOBJECTTYPE(received_host, object, "host.box") ||
            !object->object_type || value_is_uninitialized_object(object) ||
            object->status.all_type_flags != (before & ~RXFLAG_VM_OBJECT_UNINITIALIZED) ||
            object->int_value != 73 || object->num_attributes != 1 ||
            object->attributes[0]->int_value != 91 ||
            object->binary_length != sizeof(text) || memcmp(object->binary_value, text, sizeof(text))) {
            set_int(ret, 5); return;
        }
        identity = object->object_type;
        if (SETOBJECTTYPE(received_host, object, ".host..box") ||
            object->object_type != identity ||
            !SETOBJECTTYPE(received_host, object, "host.interface") ||
            !SETOBJECTTYPE(received_host, object, "host.unknown") ||
            !SETOBJECTTYPE(received_host, object, ".int") ||
            !SETOBJECTTYPE(received_host, object, NULL) ||
            !SETOBJECTTYPE(received_host, NULL, "host.box") ||
            object->object_type != identity || object->attributes[0]->int_value != 91) {
            set_int(ret, 6); return;
        }
    }
    set_int(ret, 0);
}
int main(int argc, char **paths) {
    rxvm_context context;
    rxpa_plugin_manifest_v2 modern = {
        sizeof(modern), RXPA_PLUGIN_MANIFEST_ABI_V2, "host-modern",
        all_session_capabilities, legacy_create, destroy,
        noop_session_enter, noop_session_leave, host_create
    };
    /* Deliberately poison the unavailable tail: prefix-only manifests must not
     * accidentally acquire a new factory through full-struct copying. */
    rxpa_plugin_manifest_v2 old = modern;
    value arg = {0}, ret = {0}, sig = {0};
    value *argv[1] = {&arg};
    proc_runtime *procedure;
    rxvm_context *previous;
    rxvm_memory_worker *worker;
    int failed = 0;
    old.plugin_id = "host-old";
    old.struct_size = offsetof(rxpa_plugin_manifest_v2, session_create_with_host);
    rxvm_register_static_plugin_manifest_v2(&old);
    rxvm_register_static_plugin_manifest_v2(&modern);
    rxvm_addfunc_for_plugin("host-old", probe, "host.old", "b", ".int", "text=.string");
    rxvm_addfunc_for_plugin("host-modern", probe, "host.modern", "b", ".int", "text=.string");
    rxvm_addfunc_for_plugin("host-modern", reject_uninitialized, "host.reject", "b", ".int", "object=.object");
    rxvm_addclass("host.box", "b", ".unknown");
    rxvm_addinterface("host.interface", "b", ".unknown");
    memset(&context, 0, sizeof(context)); rxinimod(&context);
    if (rxldmodp(&context) <= 0 || legacy_creates != 1 || host_creates != 1) failed = 1;
    if (!failed) {
        rxpa_host_services_v1 invalid = *received_host;
        size_t boundary;
        if (!rxpa_host_has_object_set_type(received_host) ||
            !SETOBJECTTYPE(received_host, &arg, "host.box") ||
            !SETOBJECTTYPE(NULL, &arg, "host.box")) failed = 1;
        /* Poisoned unavailable tails, including each partial pointer size. */
        for (boundary = offsetof(rxpa_host_services_v1, object_set_type);
             boundary < offsetof(rxpa_host_services_v1, object_set_type) + sizeof(invalid.object_set_type);
             ++boundary) {
            invalid.struct_size = boundary;
            if (rxpa_host_has_object_set_type(&invalid) ||
                !SETOBJECTTYPE(&invalid, &arg, "host.box")) failed = 1;
        }
        invalid = *received_host; invalid.abi_version++;
        if (rxpa_host_has_object_set_type(&invalid)) failed = 1;
        invalid = *received_host; invalid.object_set_type = NULL;
        if (rxpa_host_has_object_set_type(&invalid)) failed = 1;
        {
            size_t prefix_size = offsetof(rxpa_host_services_v1, object_set_type);
            rxpa_host_services_v1 *prefix = malloc(prefix_size);
            if (!prefix) failed = 1;
            else {
                memcpy(prefix, received_host, prefix_size);
                prefix->struct_size = prefix_size;
                if (!rxpa_host_has_string_view(prefix) ||
                    rxpa_host_has_object_set_type(prefix) ||
                    !SETOBJECTTYPE(prefix, &arg, "host.box")) failed = 1;
                free(prefix);
            }
        }
        invalid = *received_host;
        invalid.struct_size = offsetof(rxpa_host_services_v1, string_view);
        if (host_create(&invalid) || host_create(NULL)) failed = 1;
        invalid = *received_host; invalid.abi_version++;
        if (host_create(&invalid)) failed = 1;
        invalid = *received_host; invalid.string_view = NULL;
        if (host_create(&invalid)) failed = 1;
        value_init(&arg); value_init(&ret); value_init(&sig);
        worker = rxvm_memory_enter(context.worker.memory_worker);
        previous = rxvm_active_context_enter(&context);
        set_string(&arg, "A\0B\xc3\xa9", 5);
        procedure = context_find_procedure(&context, "host.modern");
        if (!procedure) failed = 1;
        else {
            rxvm_call_native_procedure(procedure, 1, argv, &ret, &sig);
            if (ret.int_value || sig.int_value) failed = 1;
        }
        /* Native boundary validation must not initialize untouched receivers
         * or their children while refreshing UTF-8 certificates. */
        procedure = context_find_procedure(&context, "host.reject");
        if (!procedure) failed = 1;
        else {
            set_string(&arg, "", 0);
            mark_value_uninitialized_object(&arg);
            set_string(arg.attributes[0], "\xc3\xa9", 2);
            mark_value_uninitialized_object(arg.attributes[0]);
            rxvm_call_native_procedure(procedure, 1, argv, &ret, &sig);
            if (sig.int_value != SIGNAL_OBJECT_NOT_INITIALIZED ||
                !value_is_uninitialized_object(&arg) ||
                !value_is_uninitialized_object(arg.attributes[0])) failed = 1;
        }
        clear_value(&arg); clear_value(&ret); clear_value(&sig);
        rxvm_active_context_leave(previous); rxvm_memory_leave(worker);
    }
    if (!failed && argc == 3) {
        int j;
        for (j = 1; j <= 2; ++j) {
            rxpa_loaded_plugin plugin;
            void *session;
            rxpa_initctx helpers;
            memset(&helpers, 0, sizeof(helpers));
            if (rxpa_open_plugin(NULL, paths[j], &plugin) || !plugin.has_manifest_v2) {
                failed = 1; break;
            }
            if (rxpa_initialize_plugin(&plugin, &helpers)) failed = 1;
            if (j == 1) {
                if (plugin.manifest_v2.session_create_with_host) failed = 1;
                session = plugin.manifest_v2.session_create();
                if (!session) failed = 1;
            } else {
                if (!plugin.manifest_v2.session_create_with_host ||
                    plugin.manifest_v2.session_create() ||
                    plugin.manifest_v2.session_create_with_host(NULL)) failed = 1;
                session = plugin.manifest_v2.session_create_with_host(received_host);
                if (session != received_host) failed = 1;
            }
            if (session) plugin.manifest_v2.session_destroy(session);
            rxpa_close_plugin(&plugin);
        }
    } else if (argc != 3) failed = 1;
    rxfremod(&context);
    if (destroys != 2) failed = 1;
    if (failed) fprintf(stderr, "FAIL: negotiated text services or old manifest compatibility\n");
    else puts("PASS: borrowed complete text, native object types, host negotiation, prefix-only static manifest and cleanup");
    return failed;
}
