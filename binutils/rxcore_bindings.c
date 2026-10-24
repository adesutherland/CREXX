/* cREXX License (MIT). Mandatory VM-core metadata, shared with the compiler. */
#include "rxcore_bindings.h"
#include "rxsignature.h"
#include "../interpreter/rxvmparse_bindings.h"
#include "../interpreter/rxvminvoke.h"
#include <string.h>

static enum rx_core_binding_class match_binding(
        const char *provider, const char *symbol, const char *options,
        const char *type, const char *args, const char *core_provider,
        const char *core_symbol, const char *core_type, const char *core_args) {
    int option_match, type_match, args_match;
    if (strcmp(symbol, core_symbol) != 0) return RX_CORE_BINDING_EXTERNAL;
    if (strcmp(provider, core_provider) != 0) return RX_CORE_BINDING_CONFLICT;
    option_match = rx_sig_provider_text_equal(options, "b");
    type_match = rx_sig_provider_text_equal(type, core_type);
    args_match = rx_sig_provider_text_equal(args, core_args);
    if (option_match < 0 || type_match < 0 || args_match < 0) return RX_CORE_BINDING_ERROR;
    return option_match && type_match && args_match
            ? RX_CORE_BINDING_MANDATORY : RX_CORE_BINDING_CONFLICT;
}

enum rx_core_binding_class rx_core_binding_classify(
        const char *provider, const char *symbol, const char *options,
        const char *type, const char *args) {
    size_t i;
    enum rx_core_binding_class result;
    if (!provider || !symbol || !options || !type || !args)
        return RX_CORE_BINDING_CONFLICT;
    for (i = 0; i < rxvm_parse_cursor_binding_count; ++i) {
        const rxvm_parse_binding *binding = &rxvm_parse_cursor_bindings[i];
        if (binding->kind != RXVM_PARSE_BIND_FUNCTION) continue;
        result = match_binding(provider, symbol, options, type, args,
                rxvm_parse_cursor_provider_id, binding->symbol, binding->type, binding->args);
        if (result != RX_CORE_BINDING_EXTERNAL) return result;
    }
    for (i = 0; i < rxvm_invoke_binding_count; ++i) {
        const rxvm_invoke_binding *binding = &rxvm_invoke_bindings[i];
        if (binding->kind != RXVM_INVOKE_BIND_FUNCTION) continue;
        result = match_binding(provider, symbol, options, type, args,
                rxvm_invoke_provider_id, binding->symbol, binding->type, binding->args);
        if (result != RX_CORE_BINDING_EXTERNAL) return result;
    }
    if (!strcmp(provider, rxvm_parse_cursor_provider_id) ||
        !strcmp(provider, rxvm_invoke_provider_id)) return RX_CORE_BINDING_CONFLICT;
    return RX_CORE_BINDING_EXTERNAL;
}
